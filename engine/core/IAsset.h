#pragma once
#include "Refs.h"

class IAsset {
public:
	int assetId = 0;

	virtual void Release() = 0;

	virtual ~IAsset() { }
};