#include "Rete.h"

#include <cassert>

void Rete::alpha_activation(AlphaMemory* alpha_node, WME* wme)
{
	alpha_node->items.push_front(wme);
	for (JoinNode* child : alpha_node->successors) child->alpha_activation(wme);
}

bool Rete::const_test_activation(ConstTestNode* const_node, WME* wme)
{
    if (const_node->field_to_test != FieldType::undefined) {
        if (wme->get_field(const_node->field_to_test) != const_node->eq_field) {
            return false;
        }
    }

    if (const_node->out_memory) {
        alpha_activation(const_node->out_memory, wme);
    }
    for (ConstTestNode* c : const_node->children) {
        const_test_activation(c, wme);
    }
    return true;
}

void Rete::addWME(WME* wme)
{
    working_memory.push_back(wme);
    const_test_activation(alpha_top, wme);
}

void Rete::update_new_node_with_matches(BetaMemory* beta)
{
    JoinNode* join = beta->parent;
    std::vector<BetaMemory*> saved_children = join->children;

    join->children = { beta };

    for (WME* item : join->alpha_mem->items) { join->alpha_activation(item); }
    join->children = saved_children;
}

BetaMemory* Rete::build_or_share_beta_memory(JoinNode* parent)
{
    for (BetaMemory* child : parent->children) { return child; }

    BetaMemory* newbeta = new BetaMemory;
    beta_memories.push_back(newbeta);
    newbeta->parent = parent;
    parent->children.push_back(newbeta);
    update_new_node_with_matches(newbeta);
    return newbeta;
}

JoinNode* Rete::build_or_share_join_node(BetaMemory* bmem, AlphaMemory* amem, std::vector<TestJoin> tests)
{
    assert(amem != nullptr);

    JoinNode* newjoin = new JoinNode;
   join_nodes.push_back(newjoin);
    newjoin->beta_mem = bmem;
    newjoin->tests = tests; newjoin->alpha_mem = amem;
    amem->successors.push_front(newjoin);
    if (bmem) { bmem->children.push_back(newjoin); }
    return newjoin;
}

void Rete::find_earlier_cond_with_field(const std::vector<Condition>& earlier_conditions, std::string required_field, int* i, int* f2)
{
    *i = earlier_conditions.size() - 1;
    *f2 = -1;
    bool found = false;
    for (auto it = earlier_conditions.rbegin(); it != earlier_conditions.rend(); ++it) {
        for (int j = 0; j < (int)FieldType::num_of_fields_type; ++j) {
            if (it->attrs[j].type != ConstOrVar::Var) continue;
            if (it->attrs[j].v == required_field) {
                *f2 = j;
                return;
            }
        }
        (*i)--;
    }
    *i = *f2 = -1;
}

std::vector<TestJoin> Rete::get_test_join_from_cond(Condition c, std::vector<Condition> earlier_conditions)
{
	std::vector<TestJoin> result;

    for (int f = 0; f < (int)FieldType::num_of_fields_type; ++f) {
        if (c.attrs[f].type != ConstOrVar::Var) continue;
        // each occurence of variable v
        const std::string v = c.attrs[f].v;
        int i, f2;
        find_earlier_cond_with_field(earlier_conditions, v, &i, &f2);
        // nothing found
        if (i == -1) { assert(f2 == -1); continue; }
        assert(i != -1); assert(f2 != -1);
        TestJoin test;
        test.field_of_arg1 = (FieldType)f;
        test.condition_number_of_arg2 = i;
        test.field_of_arg2 = (FieldType)f2;
        result.push_back(test);
    }
    return result;
}

ConstTestNode* Rete::build_or_share_constant_test_node(ConstTestNode* parent, FieldType f, std::string s)
{
    assert(parent != nullptr);

    for (ConstTestNode* child : parent->children) {
        if (child->field_to_test == f && child->eq_field == s) {
            return child;
        }
    }

    ConstTestNode* newnode = new ConstTestNode(f, s, nullptr);;
	const_test_nodes.push_back(newnode);
    parent->children.push_back(newnode);
    return newnode;
}

bool Rete::wme_passes_const_tests(WME* wme, Condition c)
{
    for (int f = 0; f < (int)FieldType::num_of_fields_type; ++f) {
        if (c.attrs[f].type != ConstOrVar::Const) continue;
        if (c.attrs[f].v != wme->fields[f]) return false;
    }
    return true;
}

AlphaMemory* Rete::build_or_share_alpha_memory_dataflow(Condition c)
{
    ConstTestNode* currentNode = alpha_top;
    for (int f = 0; f < (int)FieldType::num_of_fields_type; ++f) {
        if (c.attrs[f].type != ConstOrVar::Const) continue;
        const std::string s = c.attrs[f].v;
        currentNode = build_or_share_constant_test_node(currentNode,
            (FieldType)f, s);
    }

    if (currentNode->out_memory != nullptr) {
        return currentNode->out_memory;
    }
    assert(currentNode->out_memory == nullptr);
    currentNode->out_memory = new AlphaMemory;
   alpha_memories.push_back(currentNode->out_memory);
  
    for (WME* w : working_memory) {
       
        if (wme_passes_const_tests(w, c)) {
            alpha_activation(currentNode->out_memory, w);
        }
    }
    return currentNode->out_memory;
}

ProductionNode* Rete::addProduction(std::vector<Condition> lhs, std::string rhs)
{
	std::vector<Condition> earlierConds;

    std::vector<TestJoin> tests =
        get_test_join_from_cond(lhs[0], earlierConds);
    AlphaMemory* am = build_or_share_alpha_memory_dataflow(lhs[0]);

    BetaMemory* currentBeta = nullptr;
    JoinNode* currentJoin = build_or_share_join_node(currentBeta, am, tests);
    earlierConds.push_back(lhs[0]);

    for (int i = 1; i < lhs.size(); ++i) {
        currentBeta = build_or_share_beta_memory(currentJoin);
        tests = get_test_join_from_cond(lhs[i], earlierConds);
        am = build_or_share_alpha_memory_dataflow(lhs[i]);
        currentJoin = build_or_share_join_node(currentBeta, am, tests);
        earlierConds.push_back(lhs[i]);
    }

   
    ProductionNode* prod = new ProductionNode;
	production_nodes.push_back(prod);
    prod->parent = currentJoin; 
    printf("%s prod: %p | parent: %p\n", __FUNCTION__, prod, prod->parent);
    prod->rhs = rhs;
    currentJoin->children.push_back(prod);
  
    update_new_node_with_matches(prod);
    return prod;
}
