#pragma once
#include "BetaMemory.h"
#include <string>
#include <vector>

class ProductionNode:public BetaMemory
{
public:
	std::vector<Token*> items;
	std::string rhs;
	void join_activation(Token* t, WME* w) override;
};
