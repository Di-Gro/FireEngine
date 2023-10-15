#pragma once
#include <vector>
#include <list>
#include <string>
#include <unordered_map>

#include "CSBridge.h"

class Game;

class AssetStore {
public:
	using TypeHash = int;
	using AssetHash = int;
	using AssetsMap = std::unordered_map<TypeHash, std::vector<AssetHash>>;

	std::unordered_map<TypeHash, std::string> typeFullNames;
	std::unordered_map<TypeHash, std::string> typeNames;
	std::unordered_map<AssetHash, std::string> assetNames;

	std::vector<TypeHash> components;
	std::vector<TypeHash> assetTypes;

	AssetsMap assets;

	int actorTypeIdHash;
	int prefabTypeIdHash;
	int anyScriptIdHash = -1;
	int componentTypeIdHash;
	int sceneTypeIdHash;
	
private:
	Game* m_game;

	std::string m_emptyValue = "";
	std::string m_nullValue = "Null";
	std::string m_anyScriptName = "Asset";
	std::string m_missingValue = "Missing";
	std::string m_runtimeValue = "Runtime";

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

	const std::string& GetAssetName(int assetIdHash);
	const std::string& GetScriptName(int scriptIdHash);
	const std::string& GetScriptFullName(int scriptIdHash);
	const std::string& GetAssetGuid(int assetGuidHash);

	void SetType(TypeHash typeId, const std::string& fullName, const std::string& name);
	void AddComponent(TypeHash typeId);
	void AddAsset(TypeHash typeId, AssetHash assetId, const std::string& name);
	void AddAssetType(TypeHash typeId);

	void ClearTypes();
	void ClearComponents();
	void ClearAssets();
	void ClearAssetTypes();

	void RenameAsset(AssetHash assetId, const std::string& name);
	void RemoveAsset(TypeHash typeId, AssetHash assetId);

	std::string CreateRuntimeAssetId(const std::string& path = "");
	void AddRuntimeAssetId(const std::string& assetId);

	bool IsRuntimeAsset(int assetIdHash);
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