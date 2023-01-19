#pragma once

#include <memory>
#include <list>
#include <vector>
#include "WME.h"
#include "Token.h"

class JoinNode;

class BetaMemory {
public:
    JoinNode* parent;
    std::list<Token*> items;
    std::vector<JoinNode*> children;


    virtual void join_activation(Token* t, WME* w);
};

using BetaPtr = std::shared_ptr<BetaMemory>;

