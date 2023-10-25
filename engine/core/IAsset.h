#pragma once
#include <string>

#include "Refs.h"
#include "CSBridge.h"

class IAsset {
private:
	std::string m_assetId = "";
	int m_assetIdHash = 0;

public:
	const char* assetId() const { return m_assetId.c_str(); };
	void assetId(const char* value) { m_assetId = value; };

	int assetIdHash() const { return m_assetIdHash; }
	void assetIdHash(int value) { m_assetIdHash = value; }

	virtual void Release() = 0;
};

//PROP_GETSET_STR(IAsset, assetId);
//PROP_GETSET_CSTR(IAsset, assetId);
//PROP_GETSET(IAsset, int, assetIdHash);