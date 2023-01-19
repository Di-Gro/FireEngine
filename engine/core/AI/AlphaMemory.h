#pragma once

#include <memory>
#include <list>
#include "WME.h"
#include "JoinNode.h"

class AlphaMemory {
public:
	std::list<WME*> items;
	std::list<JoinNode*> successors;
};
using AlphaPtr = std::shared_ptr<AlphaMemory>;