#include "AssetStore.h"

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

