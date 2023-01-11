#include "Assets.h"
#include "Game.h"

//#include <monopp\mono_method_invoker.h>

std::string Assets::ShaderDefault = "../../data/engine/shaders/opaque_default.hlsl";
std::string Assets::ShaderDiffuseColor = "../../data/engine/shaders/opaque_defuse_color.hlsl";
std::string Assets::ShaderVertexColor = "../../data/engine/shaders/opaque_vertex_color.hlsl";

std::string Assets::ShaderAmbientLight = "../../data/engine/shaders/AmbientLight.hlsl";
std::string Assets::ShaderDirectionLight = "../../data/engine/shaders/DirectionLight.hlsl";
std::string Assets::ShaderPointLight = "../../data/engine/shaders/PointLight.hlsl";
std::string Assets::ShaderPointLightMesh = "../../data/engine/shaders/PointLightMesh.hlsl";
std::string Assets::ShaderSpotLight = "../../data/engine/shaders/SpotLight.hlsl";

std::string Assets::ShaderEditorHihglight = "../../data/engine/shaders/EditorHihglight.hlsl";
std::string Assets::ShaderEditorOutline = "../../data/engine/shaders/EditorOutline.hlsl";
std::string Assets::ShaderBlur = "../../data/engine/shaders/Blur.hlsl";

std::string Assets::ShaderWorldGride = "../../data/engine/shaders/WorldGride.hlsl";


void Assets::Init(Game* game) {
	m_game = game;

	auto type2 = m_game->mono()->GetType("FireYaml", "AssetStore");
	m_method_CreateAssetId = mono::make_method_invoker<unsigned int(void)>(type2, "CreateAssetIdInt");
	m_method_CreateTmpAssetId = mono::make_method_invoker<unsigned int(void)>(type2, "CreateTmpAssetIdInt");
	m_method_AddTmpAssetIdHash = mono::make_method_invoker<void(int)>(type2, "AddTmpAssetIdHash");
}

Assets::~Assets() {
	for (auto& pair : m_assets) {
		auto asset = pair.second;
		CppRefs::Remove(asset.ref);
		delete asset.ptr;
	}
}

bool Assets::Contains(int assetIdHash) {
	return m_assets.count(assetIdHash) > 0;
}

bool Assets::Contains(const std::string& assetId) {
	auto assetIdHash = GetCsHash(assetId);
	return m_assets.count(assetIdHash) > 0;
}

void Assets::Push(const std::string& assetId, IAsset* ptr) {
	auto assetIdHash = GetCsHash(assetId);
	Push(assetIdHash, ptr);
}

void Assets::Push(int pathHash, IAsset* ptr) {
	if (Contains(pathHash))
		return;

	Asset asset;
	asset.ptr = ptr;
	asset.ref = CppRefs::Create(ptr);
	
	m_assets.insert({ pathHash, asset });
}

IAsset* Assets::Pop(const std::string& assetId) {
	auto assetIdHash = GetCsHash(assetId);
	return Pop(assetIdHash);
}

IAsset* Assets::Pop(int assetIdHash) {
	if (!Contains(assetIdHash))
		return nullptr;

	auto asset = m_assets.at(assetIdHash);
	CppRefs::Remove(asset.ref);

	m_assets.erase(assetIdHash);

	if (m_dirtyAssets.contains(assetIdHash))
		m_dirtyAssets.erase(assetIdHash);
}

IAsset* Assets::Get(const std::string& assetId) {
	auto assetIdHash = GetCsHash(assetId);
	return Get(assetIdHash);
}

IAsset* Assets::Get(int assetIdHash) {
	if (!Contains(assetIdHash))
		return nullptr;

	return m_assets.at(assetIdHash).ptr;
}

bool Assets::Load(int assetIdHash, CppRef cppRef) {
	if (!Contains(assetIdHash))
		return false;

	return m_game->callbacks().loadAsset(assetIdHash, cppRef);
}

void Assets::ReloadAll() {
	for (auto& pair : m_assets) {
		auto iasset = pair.second.ptr;
		m_game->callbacks().reloadAsset(iasset->assetIdHash());
	}
}

void Assets::Reload(int assetIdHash) {
	auto iasset = Get(assetIdHash);
	if (iasset == nullptr)
		return;

	iasset->Release();
	m_game->callbacks().reloadAsset(iasset->assetIdHash());
}

void Assets::Save(int assetIdHash) {
	auto iasset = Get(assetIdHash);
	if (iasset == nullptr)
		return;

	m_game->callbacks().saveAsset(iasset->assetIdHash());

	if (m_dirtyAssets.contains(assetIdHash))
		m_dirtyAssets.erase(assetIdHash);
}

void Assets::MakeDirty(int assetIdHash) {
	m_dirtyAssets.insert(assetIdHash);
}

bool Assets::IsDirty(int assetIdHash) {
	return m_dirtyAssets.contains(assetIdHash);
}

int Assets::GetCsHash(const std::string& str) {
	int hash = m_game->callbacks().getStringHash((size_t)str.c_str());
	return hash;
}

//std::string Assets::CreateAssetId() {
//	auto id = m_method_CreateAssetId();
//	auto idStr = std::to_string(id);
//	auto nullCount = 10 - idStr.size(CreateTmpAssetId);
//	auto assetId = std::string(nullCount, '0') + idStr;
//	return assetId;
//}

std::string Assets::CreateTmpAssetId() {
	auto id = m_method_CreateTmpAssetId();
	auto idStr = std::to_string(id);
	auto nullCount = 10 - idStr.size();
	auto assetId = std::string(nullCount, '0') + idStr;
	assetId = "tmp_" + assetId;

	auto hash = GetCsHash(assetId);
	m_method_AddTmpAssetIdHash(hash);

	return assetId;
}

DEF_FUNC(Assets, Reload, void)(CppRef gameRef, int pathHash) {
	auto game = CppRefs::ThrowPointer<Game>(gameRef);
	game->assets()->Reload(pathHash);
}

DEF_FUNC(Assets, Get, CppRef)(CppRef gameRef, int pathHash) {
	auto game = CppRefs::ThrowPointer<Game>(gameRef);
	auto iasset = game->assets()->Get(pathHash);
	return CppRefs::GetRef(iasset);
}

FUNC(Assets, MakeDirty, void)(CppRef gameRef, int assetIdHash) {
	auto game = CppRefs::ThrowPointer<Game>(gameRef);
	game->assets()->MakeDirty(assetIdHash);
} 

FUNC(Assets, IsDirty, bool)(CppRef gameRef, int assetIdHash) {
	auto game = CppRefs::ThrowPointer<Game>(gameRef);
	return game->assets()->IsDirty(assetIdHash);
}