#pragma once
#include "Refs.h"

class IAsset {
public:
	int pathHash = 0;

	virtual void Release() = 0;

	virtual ~IAsset() { }
};