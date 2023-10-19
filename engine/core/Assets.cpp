#include "Assets.h"
#include "AssetStore.h"
#include "Game.h"
#include "Render.h"
#include "Forms.h"
#include "ShaderAsset.h"
#include "ImageAsset.h"
#include "TextureAsset.h"
#include "MaterialAsset.h"
#include "MeshAsset.h"
#include "MeshResource.h"

const std::string Assets::ShaderDefault = "../../engine/data/shaders/opaque_default.hlsl";
const std::string Assets::ShaderDiffuseColor = "../../engine/data/shaders/opaque_defuse_color.hlsl";
const std::string Assets::ShaderVertexColor = "../../engine/data/shaders/opaque_vertex_color.hlsl";
 
const std::string Assets::ShaderAmbientLight = "../../engine/data/shaders/AmbientLight.hlsl";
const std::string Assets::ShaderDirectionLight = "../../engine/data/shaders/DirectionLight.hlsl";
const std::string Assets::ShaderPointLight = "../../engine/data/shaders/PointLight.hlsl";
const std::string Assets::ShaderPointLightMesh = "../../engine/data/shaders/PointLightMesh.hlsl";
const std::string Assets::ShaderSpotLight = "../../engine/data/shaders/SpotLight.hlsl";
 
const std::string Assets::ShaderEditorHihglight = "../../engine/data/shaders/EditorHihglight.hlsl";
const std::string Assets::ShaderEditorOutline = "../../engine/data/shaders/EditorOutline.hlsl";
const std::string Assets::ShaderBlur = "../../engine/data/shaders/Blur.hlsl";

const std::string Assets::ShaderWorldGride = "../../engine/data/shaders/WorldGride.hlsl";
 
const std::string Assets::DefaultMaterial = "runtime:/materials/default";
const std::string Assets::DefaultDeffuseTexture = "runtime:/textures/default/deffuse";

const std::string Assets::FormBox = "runtime:/forms/box";
const std::string Assets::FormBoxLined = "runtime:/forms/linedBox";
const std::string Assets::FormSphere = "runtime:/forms/sphere";
const std::string Assets::FormSphereLined = "runtime:/forms/linedSphere";
const std::string Assets::FormCone = "runtime:/forms/cone";
const std::string Assets::FormConePath = "../../engine/data/models/cone.obj";

const std::string Assets::Img2x2rgba1111 = "runtime:/images/2x2rgba1111";
const std::string Assets::Img2x2rgba0001 = "runtime:/images/2x2rgba0001";
const std::string Assets::Img2x2rgba1001 = "runtime:/images/2x2rgba1001";


void Assets::Init(Game* game) {
	m_game = game;
}

void Assets::Start() {
	m_CreateDefaultImages();
	m_CreateDefaultMaterials();
	m_CreateDefaultMeshes();
}

void Assets::Destroy() {
	for (auto& pair : m_assets) {
		auto asset = pair.second;
		asset.iasset->Release();
	}
}

Assets::~Assets() {
	for (auto& pair : m_assets) {
		auto asset = pair.second;
		CppRefs::Remove(asset.ref);
		delete asset.iasset;
	}
}

bool Assets::Contains(int assetIdHash) {
	return m_assets.count(assetIdHash) > 0;
}

bool Assets::Contains(const std::string& assetId) {
	auto assetIdHash = GetAssetIDHash(assetId);
	return m_assets.count(assetIdHash) > 0;
}

void Assets::Push(const std::string& assetId, IAsset* iasset) {
	auto assetIdHash = GetAssetIDHash(assetId);
	if (Contains(assetIdHash))
		return;

	Asset asset;
	asset.iasset = iasset;
	asset.ref = CppRefs::Create(iasset);

	iasset->assetId(assetId.c_str());
	iasset->assetIdHash(assetIdHash);

	m_assets.insert({ assetIdHash, asset });
}

void Assets::Pop(const std::string& assetId) {
	auto assetIdHash = GetAssetIDHash(assetId);
	Pop(assetIdHash);
}

void Assets::Pop(int assetIdHash) {
	if (!Contains(assetIdHash))
		return;

	auto asset = m_assets.at(assetIdHash);

	CppRefs::Remove(asset.ref);

	m_assets.erase(assetIdHash);

	if (m_dirtyAssets.contains(assetIdHash))
		m_dirtyAssets.erase(assetIdHash);
}

IAsset* Assets::GetDinamic(const std::string& assetId) {
	auto assetIdHash = GetAssetIDHash(assetId);
	return GetDinamic(assetIdHash);
}

IAsset* Assets::GetDinamic(int assetIdHash) {
	if (!Contains(assetIdHash))
		return nullptr;

	return m_assets.at(assetIdHash).iasset;
}

bool Assets::Load(int assetIdHash, CppRef cppRef) {
	if (!Contains(assetIdHash))
		return false;

	return m_game->callbacks().loadAsset(cppRef, assetIdHash);
}

void Assets::ReloadAll() {
	for (auto& pair : m_assets) {
		auto iasset = pair.second.iasset;

		auto cppRef = CppRefs::GetRef(iasset);

		iasset->Release();
		m_game->callbacks().reloadAsset(cppRef, iasset->assetIdHash());
	}
}

void Assets::Reload(int assetIdHash) {
	auto iasset = GetDinamic(assetIdHash);
	if (iasset == nullptr)
		return;

	auto cppRef = CppRefs::GetRef(iasset);

	iasset->Release();
	m_game->callbacks().reloadAsset(cppRef, iasset->assetIdHash());
}

void Assets::Save(int assetIdHash) {
	auto iasset = GetDinamic(assetIdHash);
	if (iasset == nullptr)
		return;

	bool hasAsset = m_game->callbacks().hasAssetInStore(assetIdHash);
	if (!hasAsset) {
		std::cout << "Assets::Save() Asset not exist: assetIdHash:'" << assetIdHash << "'\n";
		return;
	}

	auto cppRef = CppRefs::GetRef(iasset);
	m_game->callbacks().saveAsset(cppRef, iasset->assetIdHash());

	if (m_dirtyAssets.contains(assetIdHash))
		m_dirtyAssets.erase(assetIdHash);
}

void Assets::MakeDirty(int assetIdHash) {
	m_dirtyAssets.insert(assetIdHash);
}

bool Assets::IsDirty(int assetIdHash) {
	return m_dirtyAssets.contains(assetIdHash);
}

int Assets::GetAssetIDHash(const std::string& str) {
	int hash = m_game->callbacks().getAssetIDHash((size_t)str.c_str());
	return hash;
}


void Assets::m_CreateDefaultImages() {
	auto* store = m_game->assetStore();

	// RUNTIME_IMG_2X2_RGBA_1111
	auto* image = new ImageAsset();
	image->resource.Init(2, 2);

	std::fill(image->resource.data, image->resource.data + image->resource.dataSize, 255);

	store->AddRuntimeAssetId(Assets::Img2x2rgba1111);
	Push(Assets::Img2x2rgba1111, image);

	// RUNTIME_IMG_2X2_RGBA_0001
	image = new ImageAsset();
	image->resource.Init(2, 2);

	for (int i = 0; i < image->resource.dataSize; i += 4) {
		image->resource.data[i] = 0;
		image->resource.data[i + 1] = 0;
		image->resource.data[i + 2] = 0;
		image->resource.data[i + 3] = 1;
	}

	store->AddRuntimeAssetId(Assets::Img2x2rgba0001);
	Push(Assets::Img2x2rgba0001, image);

	// RUNTIME_IMG_2X2_RGBA_1001
	image = new ImageAsset();
	image->resource.Init(2, 2);

	for (int i = 0; i < image->resource.dataSize; i += 4) {
		image->resource.data[i] = 1;
		image->resource.data[i + 1] = 0;
		image->resource.data[i + 2] = 0;
		image->resource.data[i + 3] = 1;
	}

	store->AddRuntimeAssetId(Assets::Img2x2rgba1001);
	Push(Assets::Img2x2rgba1001, image);
}

void Assets::m_CreateDefaultMaterials() {
	auto* render = m_game->render();
	auto* shaderAsset = m_game->shaderAsset();
	auto* store = m_game->assetStore();

	if (ImageAsset::Default == nullptr)
		ImageAsset::Default = GetStatic<ImageAsset>(Assets::Img2x2rgba1111);

	if (TextureAsset::Default == nullptr) {
		auto assetId = Assets::DefaultDeffuseTexture;

		store->AddRuntimeAssetId(assetId);
		Push(assetId, new TextureAsset());

		auto defaultTex = GetDinamic<TextureAsset>(assetId);

		defaultTex->resource = TextureResource::CreateFromImage(render, &ImageAsset::Default->resource);

		TextureAsset::Default = defaultTex;
	}
	if (MaterialAsset::Default == nullptr) {
		auto assetId = Assets::DefaultMaterial;

		store->AddRuntimeAssetId(assetId);
		Push(assetId, new MaterialAsset());

		auto mat = GetDinamic<MaterialAsset>(assetId);
		auto shaderHash = shaderAsset->GetShaderHash(Assets::ShaderDefault);

		mat->resource.Init(render);

		mat->name(assetId);
		mat->resource.shader = shaderAsset->GetShader(shaderHash);

		mat->textures.push_back(TextureAsset::Default);
		mat->resource.textures.emplace_back(ShaderResource::Create(&TextureAsset::Default->resource));

		MaterialAsset::Default = mat;
	}
}

void Assets::m_CreateDefaultMeshes() {
	auto* store = m_game->assetStore();

	auto form1 = Forms4::Box({ 1, 1, 1 }, { 1, 1, 1, 1 });
	auto form2 = Forms4::BoxLined({ 1, 1, 1 }, { 1, 1, 1, 1 });
	auto form3 = Forms4::Sphere(0.5, 12, 12, { 1, 1, 1, 1 });
	auto form4 = Forms4::SphereLined(0.5, 12, 12, { 1, 1, 1, 1 });

	MeshAsset* mesh;
	std::string assetId;

	assetId = Assets::FormBox;
	mesh = m_CreateMeshAssetFromForm(form1, assetId);
	store->AddRuntimeAssetId(assetId);
	Push(assetId, mesh);

	assetId = Assets::FormBoxLined;
	mesh = m_CreateMeshAssetFromForm(form2, assetId);
	store->AddRuntimeAssetId(assetId);
	Push(assetId, mesh);

	assetId = Assets::FormSphere;
	mesh = m_CreateMeshAssetFromForm(form3, assetId);
	store->AddRuntimeAssetId(assetId);
	Push(assetId, mesh);

	assetId = Assets::FormSphereLined;
	mesh = m_CreateMeshAssetFromForm(form4, assetId);
	store->AddRuntimeAssetId(assetId);
	Push(assetId, mesh);

	assetId = Assets::FormCone;
	mesh = new MeshAsset();
	mesh->resource = MeshResource::CreateFromObj(m_game->render(), Assets::FormConePath);
	store->AddRuntimeAssetId(assetId);
	Push(assetId, mesh);
}

MeshAsset* Assets::m_CreateMeshAssetFromForm(Forms4::Form& form, const std::string& assetId) {
	if (Contains(assetId))
		return GetDinamic<MeshAsset>(assetId);

	auto* mesh = new MeshAsset();
	auto* material = GetStatic<MaterialAsset>(Assets::DefaultMaterial);

	mesh->resource.AddShape(&form.verteces, &form.indexes, m_game->render(), 0);
	mesh->resource.topology = form.topology;
	mesh->f_staticMaterials.push_back(material);

	return mesh;
}

DEF_FUNC(Assets, Save, void)(CppRef gameRef, int assetIdHash) {
	auto game = CppRefs::ThrowPointer<Game>(gameRef);
	game->assets()->Save(assetIdHash);
}

DEF_FUNC(Assets, Reload, void)(CppRef gameRef, int pathHash) {
	auto game = CppRefs::ThrowPointer<Game>(gameRef);
	game->assets()->Reload(pathHash);
}

DEF_FUNC(Assets, Get, CppRef)(CppRef gameRef, int pathHash) {
	auto game = CppRefs::ThrowPointer<Game>(gameRef);
	auto iasset = game->assets()->GetStatic(pathHash);
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