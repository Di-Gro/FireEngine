#include "JoinNode.h"
#include <assert.h>
#include "BetaMemory.h"
#include "AlphaMemory.h"

void JoinNode::alpha_activation(WME* wme) {
    assert(alpha_mem);
    if (beta_mem) {
        for (Token* t : beta_mem->items) {
            if (!this->run_join_test(t, wme)) continue;
            for (auto child : children) child->join_activation(t, wme);
        }
    }
    else {
        for (auto child : children) { child->join_activation(nullptr, wme); }
    }
}

void JoinNode::beta_activation(Token* token) {
    assert(this->alpha_mem);
    for (WME* w : alpha_mem->items) {
        if (!this->run_join_test(token, w)) continue;
        for (BetaMemory* child : children) child->join_activation(token, w);
    }
}

bool JoinNode::run_join_test(Token* token, WME* wme) const {
    if (!beta_mem) return true;
    assert(alpha_mem);

    for (TestJoin test : tests) {
	    std::string arg1 = wme->get_field(test.field_of_arg1);
        WME* wme2 = token->index(test.condition_number_of_arg2);
	    std::string arg2 = wme2->get_field(test.field_of_arg2);
        if (arg1 != arg2) return false;
    }
    return true;
}
