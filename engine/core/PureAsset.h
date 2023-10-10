#pragma once
#include "IAsset.h"
#include "CSBridge.h"

class PureAsset : public IAsset {
public:
	void Release() override { }
};

PUSH_ASSET(PureAsset);
