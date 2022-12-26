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


void Assets::Init(Game* game) {
	m_game = game;

	auto type = m_game->mono()->GetType("Engine", "Assets");
	m_method_Load = mono::make_method_invoker<void(int)>(type, "Load");
	m_method_Reload = mono::make_method_invoker<void(int)>(type, "Reload");
	m_method_GetStringHash = mono::make_method_invoker<int(size_t, size_t)>(type, "GetStringHash");
}

Assets::~Assets() {
	for (auto& pair : m_assets) {
		auto asset = pair.second;
		CppRefs::Remove(asset.ref);
		delete asset.ptr;
	}
}

bool Assets::Contains(int assetId) {
	return m_assets.count(assetId) > 0;
}

bool Assets::Contains(const std::string& assetId) {
	auto pathHash = GetCsHash(assetId);
	return m_assets.count(pathHash) > 0;
}

void Assets::Push(const std::string& assetId, IAsset* ptr) {
	auto pathHash = GetCsHash(assetId);
	Push(pathHash, ptr);
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
	auto pathHash = GetCsHash(assetId);
	return Pop(pathHash);
}

IAsset* Assets::Pop(int assetId) {
	if (!Contains(assetId))
		return nullptr;

	auto asset = m_assets.at(assetId);
	CppRefs::Remove(asset.ref);

	m_assets.erase(assetId);
}

IAsset* Assets::Get(const std::string& assetId) {
	auto pathHash = GetCsHash(assetId);
	return Get(pathHash);
}

IAsset* Assets::Get(int assetId) {
	if (!Contains(assetId))
		return nullptr;

	return m_assets.at(assetId).ptr;
}

void Assets::ReloadAll() {
	for (auto& pair : m_assets) {
		auto iasset = pair.second.ptr;
		m_method_Reload(iasset->pathHash);
	}
}

void Assets::Reload(int assetId) {
	auto iasset = Get(assetId);
	iasset->Release();

	m_method_Reload(assetId);
}


int Assets::GetCsHash(const std::string& str) {
	auto cstr = str.c_str();
	auto ptr = &cstr[0];
	auto length = str.length();

	auto hash = m_method_GetStringHash((size_t)ptr, length);
	return hash;
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