#pragma once
#include <string>

#include "Refs.h"

class IAsset {

public:
	IAsset() { m_assetId = new char[1]{ '\0' }; }

	virtual ~IAsset() { delete[] m_assetId; }

	const char* assetId() { return m_assetId; };

	void assetId(const char* value) {
		delete[] m_assetId;
		auto length = std::string(value).size();

		m_assetId = new char[length + 1]{ '\0' };
		std::memcpy(m_assetId, value, length);
	};

	int assetIdHash() { return m_assetIdHash; }
	void assetIdHash(int value) { m_assetIdHash = value; }

	virtual void Release() = 0;

private:
	char* m_assetId = nullptr;
	int m_assetIdHash = 0;

};