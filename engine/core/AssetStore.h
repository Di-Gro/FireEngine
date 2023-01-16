#pragma once
#include <vector>
#include <list>
#include <string>
#include <unordered_map>

#include "CSBridge.h"

class Game;

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

	int actorTypeIdHash;
	int prefabTypeIdHash;
	int componentTypeIdHash;
	int sceneTypeIdHash;
	
private:
	Game* m_game;

	std::string m_emptyValue = "";
	std::string m_nullValue = "Null";
	std::string m_missingValue = "Missing";
	std::string m_dynamicValue = "Dynamic";

	std::string m_projectPath;
	std::string m_assetsPath;
	std::string m_editorPath;

	std::string m_buffer;

public:
	void Init(Game* game) { m_game = game; }
	~AssetStore();

	const std::string&	projectPath() { return m_projectPath; }
	void				projectPath(const std::string& value) { m_projectPath = value; }

	const std::string&	assetsPath() { return m_assetsPath; }
	void				assetsPath(const std::string& value) { m_assetsPath = value; }

	const std::string&	editorPath() { return m_editorPath; }
	void				editorPath(const std::string& value) { m_editorPath = value; }

	const std::string&	buffer() { return m_emptyValue; }
	void				buffer(const std::string& value) { m_buffer = value; }

	const std::string& GetAssetName(int assetIdHash) {
		if (assetIdHash == 0)
			return m_nullValue;

		if (assetIdHash == -1)
			return m_dynamicValue;

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

	const std::string& GetAssetGuid(int assetGuidHash);

	void SetType(ScriptIdHash typeId, const std::string& fullName, const std::string& name);
	void AddComponent(ScriptIdHash typeId);
	void AddAsset(ScriptIdHash typeId, AssetIdHash assetId, const std::string& name);
	void AddAssetType(ScriptIdHash typeId);

	void ClearTypes();
	void ClearComponents();
	void ClearAssets();
	void ClearAssetTypes();

	void RenameAsset(AssetIdHash assetId, const std::string& name);
	void RemoveAsset(ScriptIdHash typeId, AssetIdHash assetId);
};

FUNC(AssetStore, ClearTypes, void)(CppRef gameRef);
FUNC(AssetStore, ClearComponents, void)(CppRef gameRef);
FUNC(AssetStore, ClearAssets, void)(CppRef gameRef);
FUNC(AssetStore, ClearAssetTypes, void)(CppRef gameRef);

FUNC(AssetStore, SetType, void)(CppRef gameRef, int typeId, const char* fullName, const char* name);
FUNC(AssetStore, AddComponent, void)(CppRef gameRef, int typeId);
FUNC(AssetStore, AddAsset, void)(CppRef gameRef, int typeId, int assetId, const char* name);
FUNC(AssetStore, AddAssetType, void)(CppRef gameRef, int typeId);

PROP_GETSET_STR(AssetStore, projectPath);
PROP_GETSET_STR(AssetStore, assetsPath);
PROP_GETSET_STR(AssetStore, editorPath);

PROP_GETSET(AssetStore, int, actorTypeIdHash);
PROP_GETSET(AssetStore, int, prefabTypeIdHash);
PROP_GETSET(AssetStore, int, componentTypeIdHash);
PROP_GETSET(AssetStore, int, sceneTypeIdHash);

PROP_GETSET_STR(AssetStore, buffer);

FUNC(AssetStore, RenameAsset, void)(CppRef gameRef, int assetId, const char* name);
FUNC(AssetStore, RemoveAsset, void)(CppRef gameRef, int typeId, int assetId);