#pragma once
#include <vector>
#include <list>
#include <string>
#include <unordered_map>

#include "CSBridge.h"

class AssetStore {
public:
	using ScriptIdHash = int;
	using AssetIdHash = int;
	
	std::unordered_map<ScriptIdHash, std::string> typeFullNames;
	std::unordered_map<ScriptIdHash, std::string> typeNames;
	std::unordered_map<AssetIdHash, std::string> assetNames;

	std::vector<ScriptIdHash> components;
	std::vector<ScriptIdHash> assetTypes;

	std::unordered_map<ScriptIdHash, std::vector<AssetIdHash>> assets;
	
private:
	std::string m_emptyValue = "";
	std::string m_nullValue = "Null";
	std::string m_missingValue = "Missing";

public:
	~AssetStore();

	const std::string& GetAssetName(int assetIdHash) {
		if (assetIdHash == 0)
			return m_nullValue;

		if (assetNames.contains(assetIdHash))
			return assetNames[assetIdHash];

		return m_missingValue;
	}

	const std::string& GetScriptName(int scriptIdHash) {
		if (scriptIdHash == 0)
			return m_nullValue;

		if (typeNames.contains(scriptIdHash))
			return typeNames[scriptIdHash];

		return m_missingValue;
	}

	const std::string& GetScriptFullName(int scriptIdHash) {
		if (scriptIdHash == 0)
			return m_nullValue;

		if (typeFullNames.contains(scriptIdHash))
			return typeFullNames[scriptIdHash];

		return m_missingValue;
	}

	void SetType(ScriptIdHash typeId, const std::string& fullName, const std::string& name);
	void AddComponent(ScriptIdHash typeId);
	void AddAsset(ScriptIdHash typeId, AssetIdHash assetId, const std::string& name);
	void AddAssetType(ScriptIdHash typeId);

	void ClearTypes();
	void ClearComponents();
	void ClearAssets();
	void ClearAssetTypes();
};

FUNC(AssetStore, ClearTypes, void)(CppRef gameRef);
FUNC(AssetStore, ClearComponents, void)(CppRef gameRef);
FUNC(AssetStore, ClearAssets, void)(CppRef gameRef);
FUNC(AssetStore, ClearAssetTypes, void)(CppRef gameRef);

FUNC(AssetStore, SetType, void)(CppRef gameRef, int typeId, const char* fullName, const char* name);
FUNC(AssetStore, AddComponent, void)(CppRef gameRef, int typeId);
FUNC(AssetStore, AddAsset, void)(CppRef gameRef, int typeId, int assetId, const char* name);
FUNC(AssetStore, AddAssetType, void)(CppRef gameRef, int typeId);