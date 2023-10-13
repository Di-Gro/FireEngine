#include "MeshAssets.h"

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <filesystem>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include "json.hpp"

#include "Game.h"
#include "Scene.h"
#include "Render.h"
#include "Assets.h"
#include "Actor.h"

#include "ShaderAsset.h"
#include "TextureAsset.h"
#include "MaterialAsset.h"
#include "ImageAsset.h"
#include "ImageAssets.h"

#include "MeshComponent.h"
#include "Forms.h"
#include "Random.h"

namespace fs = std::filesystem;

std::string MeshAssets::materialDefault = "M_Default";

std::string MeshAssets::formBox = "runtime:/form/Box";
std::string MeshAssets::formBoxLined = "runtime:/form/BoxLined";
std::string MeshAssets::formSphere = "runtime:/form/Sphere";
std::string MeshAssets::formSphereLined = "runtime:/form/SphereLined";
std::string MeshAssets::formCone = "../../engine/data/models/cone.obj";
//std::string MeshAssets::formCapsule = "../../engine/data/models/capsule.obj";



int MeshAssets::CreateHash(const fs::path& path) {
	auto pathStr = path.string();
	auto hash = m_game->assets()->GetCsAssetIDHash(pathStr);

	m_assetPaths[hash] = pathStr;

	return hash;
}

void MeshAssets::Init(Game* game) {
	m_game = game;
	InitMono();
}

void MeshAssets::Start() {
	m_InitDefaultMaterials();
	m_InitDefaultStaticMeshes();
}

void MeshAssets::InitMono() {
	auto ref = CppRefs::Create(this);
	m_game->callbacks().setMeshAssetRef(RefCpp(ref.cppRef()));
}

void MeshAssets::Destroy() {
	//for (auto& hash_asset : m_assets)
	//	delete hash_asset.second;

	for (auto it = m_materials.begin(); it != m_materials.end(); it++) {
		auto* material = it->second;
		m_DeleteMaterialAsset(material);
	}

	for (auto it = m_dynamicMaterials.begin(); it != m_dynamicMaterials.end(); it++) {
		auto* material = it->second;
		m_DeleteMaterialAsset(material);
	}
}

MaterialAsset* MeshAssets::m_NewMaterialAsset(const std::string& assetId) {
	auto assetIdHash = m_game->assets()->GetCsAssetIDHash(assetId);
	auto cppRef = MaterialAsset_PushAsset(CppRefs::GetRef(m_game), assetId.c_str(), assetIdHash);
	auto material = CppRefs::ThrowPointer<MaterialAsset>(cppRef);

	material->resource.Init(m_game->render());

	return material;
}

void MeshAssets::m_DeleteMaterialAsset(MaterialAsset* material) {
	m_game->assets()->Pop(material->assetIdHash());
	delete material;
}

TextureAsset* MeshAssets::m_NewTextureAsset(const std::string& assetId) {
	auto assetIdHash = m_game->assets()->GetCsAssetIDHash(assetId);
	auto texCppRef = TextureAsset_PushAsset(CppRefs::GetRef(m_game), assetId.c_str(), assetIdHash);

	return CppRefs::ThrowPointer<TextureAsset>(texCppRef);
}

MeshAsset* MeshAssets::m_NewMeshAsset(const std::string& assetId) {
	auto gameRef = CppRefs::GetRef(m_game);
	auto assetIdHash = m_game->assets()->GetCsAssetIDHash(assetId);
	auto meshRef = MeshAsset_PushAsset(gameRef, assetId.c_str(), assetIdHash);

	return CppRefs::ThrowPointer<MeshAsset>(meshRef);
}

void MeshAssets::Load(const fs::path& path) {
	m_Load(CreateHash(path), path);
}

void MeshAssets::Load(int hash) {
	auto path = m_assetPaths.at(hash);
	m_Load(hash, path);
}

void MeshAssets::m_Load(int hash, const fs::path& path) {
	if (m_game->assets()->Contains(hash))
		return;

	m_CreateMeshAsset(hash, path);
}

const MeshAsset* MeshAssets::GetMesh(const fs::path& path) {
	auto hash = CreateHash(path);

	if (!m_game->assets()->Contains(hash))
		m_Load(hash, path);

	return (MeshAsset*)m_game->assets()->Get(hash);
}

const MeshAsset* MeshAssets::GetMesh(int hash) {
	if (!m_game->assets()->Contains(hash)) {
		auto path = m_assetPaths.at(hash);
		m_Load(hash, path);
	}
	return (MeshAsset*)m_game->assets()->Get(hash);
}

//Mesh4* MeshAssets::m_GetMeshMutable(fs::path path) {
//	auto hash = CreateHash(path);
//
//	m_Load(hash, path);
//	return(Mesh4*)m_game->assets()->Get(hash);
//}

const std::vector<const MaterialAsset*>* MeshAssets::GetMaterials(const fs::path& path) {
	auto hash = CreateHash(path);
	
	m_Load(hash, path);

	auto mesh = (MeshAsset*)m_game->assets()->Get(hash);
	return mesh->GetMaterials();
}

const std::vector<const MaterialAsset*>* MeshAssets::GetMaterials(const MeshAsset* mesh) {
	return mesh->GetMaterials();
}

//void MeshAssets::InitMesh(MeshResource* mesh, const fs::path& path) {
//	auto dir = path.parent_path().string();
//
//	tinyobj::attrib_t attrib;
//	std::vector<tinyobj::shape_t> shapes;
//	std::vector<tinyobj::material_t> materials;
//
//	std::string warn, err;
//	bool res = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.string().c_str(), dir.c_str());
//	assert(res);
//
//	m_InitMesh(mesh, attrib, shapes);
//
//	if (err != "")
//		std::cout << err << std::endl;
//
//	//if (warn != "")
//	//	std::cout << warn << std::endl;
//}

MeshAsset* MeshAssets::m_CreateMeshAsset(int hash, const fs::path& path) {
	auto dir = path.parent_path().string();	

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string warn, err;
	bool res = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.string().c_str(), dir.c_str());
	assert(res);

	auto assetId = path.string();
	auto mesh = m_NewMeshAsset(assetId);

	mesh->resource = MeshResource::CreateFromObj(m_game->render(), path);
	//m_InitMesh(mesh, attrib, shapes);

	m_InitMaterials(mesh, materials, dir);

	if (err != "")
		std::cout << err << std::endl;

	//if (warn != "")
	//	std::cout << warn << std::endl;

	return mesh;
}

void MeshAssets::ReloadMaterials() {
	//TODO: Reload Materials
	//for (auto& hash_asset : m_assets) {
	//	auto asset = hash_asset.second;
	//	auto path = asset->path.string();
	//	auto dir = asset->path.parent_path().string();

	//	tinyobj::attrib_t attrib;
	//	std::vector<tinyobj::shape_t> shapes;
	//	std::vector<tinyobj::material_t> materials;

	//	std::string warn, err;
	//	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str(), dir.c_str())) {
	//		std::cout << err << std::endl;
	//		return;
	//	}

	//	//asset->materials.clear();
	//	asset->staticMaterials.clear();
	//	m_InitMaterials(asset, materials, dir);
	//}
}

MaterialAsset* MeshAssets::CreateDynamicMaterial(const std::string& name, const fs::path& shaderPath) {
	auto* render = m_game->render();
	//auto* images = m_game->imageAsset();
	auto* shaderAsset = m_game->shaderAsset();

	auto matAssetId = name + std::to_string(Random().Int());
	auto mat = m_NewMaterialAsset(matAssetId);

	mat->isDynamic = true;
	mat->name(name);
	mat->resource.shader = shaderAsset->GetShader(shaderAsset->GetShaderHash(shaderPath));

	const auto* image = m_game->imageAsset()->Get(ImageAssets::RUNTIME_IMG_2X2_RGBA_1111);

	auto texAssetId = name + "deffuseTex" + std::to_string(Random().Int());
	auto deffuseTex = m_NewTextureAsset(texAssetId);

	deffuseTex->resource = TextureResource::CreateFromImage(render, &image->resource);

	mat->textures.push_back(deffuseTex);
	mat->resource.textures.emplace_back(ShaderResource::Create(&deffuseTex->resource));

	m_dynamicMaterials.insert({ CppRefs::GetRef((void*)mat), mat });
	return mat;
}

MaterialAsset* MeshAssets::CreateDynamicMaterial(const MaterialAsset* otherRes) {
	auto* render = m_game->render();
	auto* images = m_game->imageAsset();

	auto mat = m_NewMaterialAsset(m_game->assets()->CreateTmpAssetId());
	mat->resource.priority = otherRes->resource.priority;
	mat->isDynamic = true;
	mat->name(otherRes->name());
	mat->resource.shader = otherRes->resource.shader;
	mat->resource.data = otherRes->resource.data;
	
	for (int i = 0; i < otherRes->textures.size(); i++) {
		auto& otherTex = otherRes->textures[i];

		const auto* image = otherTex->image;
		assert(image != nullptr);

		auto assetId = otherTex->name + "deffuseTex" + std::to_string(Random().Int());
		auto thisTex = m_NewTextureAsset(assetId);

		thisTex->resource = TextureResource::CreateFromImage(render, &image->resource);

		mat->textures.push_back(thisTex);
		mat->resource.textures.emplace_back(ShaderResource::Create(&thisTex->resource));

		thisTex->name = otherTex->name;
	}

	m_dynamicMaterials.insert({ CppRefs::GetRef((void*)mat), mat});
	return mat;
}

void MeshAssets::DeleteDynamicMaterial(MaterialAsset* mat) {
	auto matRef = CppRefs::GetRef((void*)mat);
	if (m_dynamicMaterials.count(matRef) == 0)
		return;

	m_game->DeleteMaterialFromAllScenes(&mat->resource);

	m_dynamicMaterials.erase(matRef);
	m_DeleteMaterialAsset(mat);
}
//
//void MeshAssets::m_InitMesh(
//	MeshResource* mesh,
//	const tinyobj::attrib_t& attrib,
//	const std::vector<tinyobj::shape_t>& shapes)
//{
//	for (auto& shape : shapes) {
//		m_InitShape(mesh, attrib, shape);
//	}
//}
//
//void MeshAssets::m_InitShape(
//	MeshResource* mesh,
//	const tinyobj::attrib_t& attrib,
//	const tinyobj::shape_t& shape)
//{
//	auto& verts = attrib.vertices;
//
//	std::vector<Vertex> verteces;
//	std::vector<int> indeces;
//
//	verteces.reserve(verts.size() / 3);
//	indeces.reserve(shape.mesh.indices.size());
//
//	int faceSize = 3;
//	for (int i = 0; i < shape.mesh.indices.size(); i += faceSize) {
//		auto vertex1 = m_ReadVertex(attrib, shape.mesh.indices[i + 0]);
//		auto vertex2 = m_ReadVertex(attrib, shape.mesh.indices[i + 1]);
//		auto vertex3 = m_ReadVertex(attrib, shape.mesh.indices[i + 2]);
//
//		verteces.insert(verteces.end(), { vertex1 , vertex2, vertex3 });
//		indeces.insert(indeces.end(), { i + 2, i + 1, i + 0 });
//	}
//
//	int matIndex = shape.mesh.material_ids[0];
//	matIndex = matIndex >= 0 ? matIndex : 0;
//
//	mesh->resource.AddShape(&verteces, &indeces, m_game->render(), matIndex);
//}

//Vertex MeshAssets::m_ReadVertex(const tinyobj::attrib_t& attrib, const tinyobj::index_t& index) {
//	auto& verts = attrib.vertices;
//	auto& colors = attrib.colors;
//	auto& normals = attrib.normals;
//	auto& texcoords = attrib.texcoords;
//
//	Vertex vertex;
//
//	auto v = (size_t)index.vertex_index * 3;
//	auto n = (size_t)index.normal_index * 3;
//	auto t = (size_t)index.texcoord_index * 2;
//
//	if (index.vertex_index >= 0) {
//		vertex.position = { verts[v], verts[v + 1], verts[v + 2], 0.0f };
//		vertex.color = { colors[v], colors[v + 1], colors[v + 2], 1.0f };
//	}
//	if (index.normal_index >= 0)
//		vertex.normal = { normals[n], normals[n + 1], normals[n + 2], 1.0f };
//
//	if (index.texcoord_index >= 0)
//		vertex.uv = { texcoords[t], texcoords[t + 1] };
//
//	return vertex;
//}

void MeshAssets::m_InitDefaultMaterials() {
	auto* render = m_game->render();
	auto* images = m_game->imageAsset();
	const auto* shader = m_game->shaderAsset()->GetShader(m_game->shaderAsset()->GetShaderHash(Assets::ShaderDefault));

	auto hash = std::hash<std::string>()(materialDefault);

	if (ImageAsset::Default == nullptr)
		ImageAsset::Default = images->Get(ImageAssets::RUNTIME_IMG_2X2_RGBA_1111);

	if (TextureAsset::Default == nullptr) {
		auto assetId = materialDefault + "deffuseTex";
		auto defaultTex = m_NewTextureAsset(assetId);

		defaultTex->resource = TextureResource::CreateFromImage(render, &ImageAsset::Default->resource);

		TextureAsset::Default = defaultTex;
	}
	if (m_materials.count(hash) == 0) {
		auto* mat = m_NewMaterialAsset(m_game->assets()->CreateTmpAssetId());
		m_materials.insert({ hash, mat });

		mat->name(materialDefault);
		mat->resource.shader = shader;

		mat->textures.push_back(TextureAsset::Default);
		mat->resource.textures.emplace_back(ShaderResource::Create(&TextureAsset::Default->resource));
	}
}

void MeshAssets::m_InitDefaultStaticMeshes() {
	auto form1 = Forms4::Box({ 1, 1, 1 }, { 1, 1, 1, 1 });
	auto form2 = Forms4::BoxLined({ 1, 1, 1 }, { 1, 1, 1, 1 });
	auto form3 = Forms4::Sphere(0.5, 12, 12, { 1, 1, 1, 1 });
	auto form4 = Forms4::SphereLined(0.5, 12, 12, { 1, 1, 1, 1 });

	m_InitAssetFromForm(form1, MeshAssets::formBox);
	m_InitAssetFromForm(form2, MeshAssets::formBoxLined);
	m_InitAssetFromForm(form3, MeshAssets::formSphere);
	m_InitAssetFromForm(form4, MeshAssets::formSphereLined);
}

void MeshAssets::m_InitAssetFromForm(Forms4::Form& form, const fs::path& path) {
	auto hash = CreateHash(path);
	auto material = GetStaticMaterial(MeshAssets::materialDefault);

	auto assetId = path.string();
	auto mesh = m_NewMeshAsset(assetId);

	mesh->resource.AddShape(&form.verteces, &form.indexes, m_game->render(), 0);
	mesh->resource.topology = form.topology;
	mesh->f_staticMaterials.push_back(material);
}

void MeshAssets::m_InitMaterials(
	MeshAsset* mesh,
	const std::vector<tinyobj::material_t>& materials,
	const std::string& directory)
{
	auto* render = m_game->render();
	auto* images = m_game->imageAsset();
	const auto* shader = m_game->shaderAsset()->GetShader(m_game->shaderAsset()->GetShaderHash(Assets::ShaderDefault));

	if (materials.empty()) {
		auto* mat = GetStaticMaterial(MeshAssets::materialDefault);
		mesh->f_staticMaterials.push_back(mat);
	}
	for (auto& tinyMat : materials) {
		auto mat = GetStaticMaterial(tinyMat, directory);
		mesh->f_staticMaterials.push_back(mat);
	}
}

const MaterialAsset* MeshAssets::GetStaticMaterial(const std::string& name) {
	auto hash = std::hash<std::string>()(name);

	if (m_materials.count(hash) > 0)
		return m_materials.at(hash);

	return nullptr;
}

const MaterialAsset* MeshAssets::GetStaticMaterial(const tinyobj::material_t& tinyMat, const std::string& directory) {
	auto hash = std::hash<std::string>()(tinyMat.name);

	if (m_materials.count(hash) > 0)
		return m_materials.at(hash);

	m_LoadMaterial(hash, tinyMat, directory);
	return m_materials.at(hash);
}

MaterialAsset* MeshAssets::m_LoadMaterial(
	int hash,
	const tinyobj::material_t& tinyMat,
	const std::string& directory)
{
	auto* render = m_game->render();
	auto* images = m_game->imageAsset();
	const auto* shader = m_game->shaderAsset()->GetShader(m_game->shaderAsset()->GetShaderHash(Assets::ShaderDefault));

	auto material = m_NewMaterialAsset(m_game->assets()->CreateTmpAssetId());
	m_materials.insert({ hash, material });

	auto& mat = *material;
	mat.name(tinyMat.name);
	mat.resource.shader = shader;

	mat.resource.data.diffuse = tinyMat.diffuse[0];
	mat.resource.data.ambient = tinyMat.ambient[0];
	mat.resource.data.specular = tinyMat.specular[0];
	mat.resource.data.shininess = tinyMat.shininess;

	auto& dc = tinyMat.diffuse;
	mat.resource.data.diffuseColor = Vector3(dc[0], dc[1], dc[2]);

	auto assetId = directory + "/" + tinyMat.diffuse_texname;
	auto deffuseTex = m_NewTextureAsset(assetId);
	mat.textures.push_back(deffuseTex);

	if (tinyMat.diffuse_texname != "") {
		deffuseTex->name = directory + "/" + tinyMat.diffuse_texname;

		if (!fs::exists(deffuseTex->name)) {
			std::cout << "Warning! MeshAssets: Image not found (" << deffuseTex->name << ") " << std::endl;
			deffuseTex->name.clear();
		}
	}

	const auto* image = deffuseTex->name.empty()
		? images->Get(ImageAssets::RUNTIME_IMG_2X2_RGBA_1111)
		: images->Get(deffuseTex->name);

	std::string name = deffuseTex->name;
	deffuseTex->resource = TextureResource::CreateFromImage(render, &image->resource);
	deffuseTex->name = name;

	mat.resource.textures.emplace_back(ShaderResource::Create(&deffuseTex->resource));

	return material;
}

FUNC(MeshAssets, Load, void)(CppRef meshAssetRef, int assetHash) {
	CppRefs::ThrowPointer<MeshAssets>(meshAssetRef)->Load(assetHash);
}

DEF_FUNC(MeshAssets, GetMesh, CppRef)(CppRef meshAssetRef, int assetHash) {
	auto mesh = CppRefs::ThrowPointer<MeshAssets>(meshAssetRef)->GetMesh(assetHash);
	return CppRefs::GetRef((void*)mesh);
}

DEF_FUNC(MeshAssets, CreateDynamicMaterial, CppRef)(CppRef meshAssetRef, CppRef otherMaterialRef) {
	auto material = CppRefs::ThrowPointer<MaterialAsset>(otherMaterialRef);
	auto meshAsset = CppRefs::ThrowPointer<MeshAssets>(meshAssetRef);

	auto newMaterial = meshAsset->CreateDynamicMaterial(material);

	return CppRefs::GetRef(newMaterial);
}

DEF_FUNC(MeshAssets, DeleteDynamicMaterial, void)(CppRef meshAssetRef, CppRef otherMaterialRef) {
	auto material = CppRefs::ThrowPointer<MaterialAsset>(otherMaterialRef);
	auto meshAsset = CppRefs::ThrowPointer<MeshAssets>(meshAssetRef);

	meshAsset->DeleteDynamicMaterial(material);
}