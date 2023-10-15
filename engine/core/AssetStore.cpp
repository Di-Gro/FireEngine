#include "AssetStore.h"
#include <unordered_map>
#include "Game.h"
#include "Assets.h"
#include "Random.h"

AssetStore::~AssetStore() { }

const std::string& AssetStore::GetAssetName(int assetIdHash) {
	if (assetIdHash == 0)
		return m_nullValue;

	// assetIdHash == -1
	if (IsRuntimeAsset(assetIdHash)) 
		return m_runtimeValue;

	if (assetNames.contains(assetIdHash))
		return assetNames[assetIdHash];

	return m_missingValue;
}

const std::string& AssetStore::GetScriptName(int scriptIdHash) {
	if (scriptIdHash == 0)
		return m_nullValue;

	if (scriptIdHash == anyScriptIdHash)
		return m_anyScriptName;

	if (typeNames.contains(scriptIdHash))
		return typeNames[scriptIdHash];

	return m_missingValue;
}

const std::string& AssetStore::GetScriptFullName(int scriptIdHash) {
	if (scriptIdHash == 0)
		return m_nullValue;

	if (typeFullNames.contains(scriptIdHash))
		return typeFullNames[scriptIdHash];

	return m_missingValue;
}

void AssetStore::SetType(TypeHash typeId, const std::string& fullName, const std::string& name) {
	typeFullNames[typeId] = fullName;
	typeNames[typeId] = name;
}

void AssetStore::AddComponent(TypeHash typeId) {
	components.push_back(typeId);
}

void AssetStore::AddAsset(TypeHash typeId, AssetHash assetId, const std::string& name) {
	assetNames[assetId] = name;
	assets[typeId].push_back(assetId);
}

void AssetStore::RenameAsset(AssetHash assetId, const std::string& name) {
	assetNames[assetId] = name;
}

void AssetStore::RemoveAsset(TypeHash typeId, AssetHash assetId) {
	
	auto& list = assets.at(typeId);
	for (int i = 0; i < list.size(); i++) {
		if (list[i] == assetId) {
			list.erase(list.begin() + i);
			return;
		}
	}
}

void AssetStore::AddAssetType(TypeHash typeId) {
	assetTypes.push_back(typeId);
}

void AssetStore::ClearTypes() {
	typeFullNames.clear();
	typeNames.clear();
}

void AssetStore::ClearComponents() {
	components.clear();
}

void AssetStore::ClearAssets() {
	assetNames.clear();
	assets.clear();
}

void AssetStore::ClearAssetTypes() {
	assetTypes.clear();
}

const std::string& AssetStore::GetAssetGuid(int assetGuidHash) {
	m_game->callbacks().requestAssetGuid(assetGuidHash);
	return m_buffer;
}


std::string AssetStore::CreateRuntimeAssetId(const std::string& name) {
	auto sep = name == "" ? "" : "/";
	auto assetId = "runtime:/" + name + sep + std::to_string(Random().Int());

	AddRuntimeAssetId(assetId);

	return assetId;
}

void AssetStore::AddRuntimeAssetId(const std::string& assetId) {
	m_game->callbacks().addRuntimeAsset(m_game->assets()->GetAssetIDHash(assetId));
}

bool AssetStore::IsRuntimeAsset(int assetIdHash) {
	return m_game->callbacks().isRuntimeAsset(assetIdHash);
}

DEF_FUNC(AssetStore, ClearTypes, void)(CppRef gameRef) {
	auto game = CppRefs::ThrowPointer<Game>(gameRef);
	game->assetStore()->ClearTypes();
}

DEF_FUNC(AssetStore, ClearComponents, void)(CppRef gameRef) {
	auto game = CppRefs::ThrowPointer<Game>(gameRef);
	game->assetStore()->ClearComponents();
}

DEF_FUNC(AssetStore, ClearAssets, void)(CppRef gameRef) {
	auto game = CppRefs::ThrowPointer<Game>(gameRef);
	game->assetStore()->ClearAssets();
}

DEF_FUNC(AssetStore, ClearAssetTypes, void)(CppRef gameRef) {
	auto game = CppRefs::ThrowPointer<Game>(gameRef);
	game->assetStore()->ClearAssetTypes();
}

DEF_FUNC(AssetStore, SetType, void)(CppRef gameRef, int typeId, const char* fullName, const char* name) {
	auto game = CppRefs::ThrowPointer<Game>(gameRef);
	game->assetStore()->SetType(typeId, fullName, name);
}

DEF_FUNC(AssetStore, AddComponent, void)(CppRef gameRef, int typeId) {
	auto game = CppRefs::ThrowPointer<Game>(gameRef);
	game->assetStore()->AddComponent(typeId);
}

DEF_FUNC(AssetStore, AddAsset, void)(CppRef gameRef, int typeId, int assetId, const char* name) {
	auto game = CppRefs::ThrowPointer<Game>(gameRef);
	game->assetStore()->AddAsset(typeId, assetId, name);
}

DEF_FUNC(AssetStore, AddAssetType, void)(CppRef gameRef, int typeId) {
	auto game = CppRefs::ThrowPointer<Game>(gameRef);
	game->assetStore()->AddAssetType(typeId);
}

DEF_PROP_GETSET_STR(AssetStore, projectPath);
DEF_PROP_GETSET_STR(AssetStore, assetsPath);
DEF_PROP_GETSET_STR(AssetStore, editorPath);

DEF_PROP_GETSET_F(AssetStore, int, actorTypeIdHash, actorTypeIdHash);
DEF_PROP_GETSET_F(AssetStore, int, prefabTypeIdHash, prefabTypeIdHash);
DEF_PROP_GETSET_F(AssetStore, int, componentTypeIdHash, componentTypeIdHash);
DEF_PROP_GETSET_F(AssetStore, int, sceneTypeIdHash, sceneTypeIdHash);

DEF_PROP_GETSET_STR(AssetStore, buffer);

FUNC(AssetStore, RenameAsset, void)(CppRef gameRef, int assetId, const char* name) {
	auto game = CppRefs::ThrowPointer<Game>(gameRef);
	game->assetStore()->RenameAsset(assetId, name);
}
FUNC(AssetStore, RemoveAsset, void)(CppRef gameRef, int typeId, int assetId) {
	auto game = CppRefs::ThrowPointer<Game>(gameRef);
	game->assetStore()->RemoveAsset(typeId, assetId);
}