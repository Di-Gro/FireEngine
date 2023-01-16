#pragma once
#include "ConstTestNode.h"
#include "AlphaMemory.h"
#include "BetaMemory.h"
#include "JoinNode.h"
#include "ProductionNode.h"
#include "Condition.h"
#include "WME.h"

#include <vector>




class Rete
{
public:
	ConstTestNode* alpha_top;
	std::vector<AlphaMemory*> alpha_memories;
	std::vector<BetaMemory*> beta_memories;
	std::vector<ConstTestNode*> const_test_nodes;
	std::vector<JoinNode*> join_nodes;
	std::vector<ProductionNode*> production_nodes;
	std::vector<WME*> working_memory;

public:
	void alpha_activation(AlphaMemory* alpha_node, WME* wme);
	bool const_test_activation(ConstTestNode* const_node, WME* wme);
	void addWME(WME* wme);
	void updateWME(WME* wme);
	void removeWME(WME* wme);
	void update_new_node_with_matches(BetaMemory* beta);
	BetaMemory* build_or_share_beta_memory(JoinNode* parent);
	JoinNode* build_or_share_join_node(BetaMemory* bmem, AlphaMemory* amem, std::vector<TestJoin> tests);
	void find_earlier_cond_with_field(const std::vector<Condition>& earlier_conditions, std::string required_field, int* i, int* f2);
	std::vector<TestJoin> get_test_join_from_cond(Condition c, std::vector<Condition> earlier_conditions);
	ConstTestNode* build_or_share_constant_test_node(ConstTestNode* parent, FieldType f, std::string s);
	bool wme_passes_const_tests(WME* wme, Condition c);
	AlphaMemory* build_or_share_alpha_memory_dataflow(Condition c);
	ProductionNode* addProduction(std::vector<Condition> lhs, std::string rhs);
};
