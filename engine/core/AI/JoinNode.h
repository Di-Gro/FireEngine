#pragma once


#include "Token.h"
#include "TestJoin.h"
#include <vector>

class AlphaMemory;
class BetaMemory;

class JoinNode {
public:
    AlphaMemory* alpha_mem;
    BetaMemory* beta_mem;

    std::vector<BetaMemory*> children;
    std::vector<TestJoin> tests;

    void alpha_activation(WME* wme);
    void beta_activation(Token* token);
    bool run_join_test(Token* token, WME* wme) const;
};

