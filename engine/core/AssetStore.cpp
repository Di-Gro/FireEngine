#include "AssetStore.h"
#include <unordered_map>
#include "Game.h"

AssetStore::~AssetStore() { }

void AssetStore::SetType(ScriptIdHash typeId, const std::string& fullName, const std::string& name) {
	typeFullNames[typeId] = fullName;
	typeNames[typeId] = name;
}

void AssetStore::AddComponent(ScriptIdHash typeId) {
	components.push_back(typeId);
}

void AssetStore::AddAsset(ScriptIdHash typeId, AssetIdHash assetId, const std::string& name) {
	assetNames[assetId] = name;
	assets[typeId].push_back(assetId);
}

void AssetStore::RenameAsset(AssetIdHash assetId, const std::string& name) {
	assetNames[assetId] = name;
}

void AssetStore::RemoveAsset(ScriptIdHash typeId, AssetIdHash assetId) {
	
	auto& list = assets.at(typeId);
	for (int i = 0; i < list.size(); i++) {
		if (list[i] == assetId) {
			list.erase(list.begin() + i);
			return;
		}
	}
}

void AssetStore::AddAssetType(ScriptIdHash typeId) {
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

FUNC(AssetStore, RenameAsset, void)(CppRef gameRef, int assetId, const char* name) {
	auto game = CppRefs::ThrowPointer<Game>(gameRef);
	game->assetStore()->RenameAsset(assetId, name);
}
FUNC(AssetStore, RemoveAsset, void)(CppRef gameRef, int typeId, int assetId) {
	auto game = CppRefs::ThrowPointer<Game>(gameRef);
	game->assetStore()->RemoveAsset(typeId, assetId);
}