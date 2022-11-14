#include "MeshAsset.h"

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <map>


#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include "json.hpp"

#include "Game.h"
#include "Material.h"

#include "MeshComponent.h"
#include "Attachable.h"


unsigned int MeshAsset::GetHash(fs::path path) {
	return std::hash<std::string>()(path.string());
}

void MeshAsset::Asset::InitConstMaterials() {
	constMaterials.reserve(materials.size());
	for (auto& mat : materials)
		constMaterials.push_back(&mat);
}

MeshAsset::~MeshAsset() {
	for (auto& hash_asset : m_assets)
		delete hash_asset.second;
}

void MeshAsset::Load(fs::path path) {
	m_Load(GetHash(path), path);
}

void MeshAsset::m_Load(unsigned int hash, fs::path path) {
	if (m_assets.count(hash) > 0)
		return;

	auto* asset = m_CreateMeshAsset(path);
	m_assets.insert({ hash, asset });
}

const Mesh4* MeshAsset::GetMesh(fs::path path) {
	auto hash = GetHash(path);

	m_Load(hash, path);
	return &m_assets.at(hash)->mesh;
}

Mesh4* MeshAsset::m_GetMeshMutable(fs::path path) {
	auto hash = GetHash(path);

	m_Load(hash, path);
	return &m_assets.at(hash)->mesh;
}

const std::vector<const Material*>* MeshAsset::GetMaterials(fs::path path) {
	auto hash = GetHash(path);

	m_Load(hash, path);
	return &m_assets.at(hash)->constMaterials;
}

MeshAsset::Asset* MeshAsset::m_CreateMeshAsset(fs::path path) {
	Asset* asset = nullptr;

	auto dir = path.parent_path().string();

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string warn, err;
	assert(tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.string().c_str(), dir.c_str()));

	asset = new Asset();
	asset->path = path;

	m_InitMesh(asset, attrib, shapes);
	m_InitMaterials(asset, materials, dir);

	if (err != "")
		std::cout << err << std::endl;

	if (warn != "")
		std::cout << warn << std::endl;

	return asset;
}

void MeshAsset::ReloadMaterials() {
	for (auto& hash_asset : m_assets) {
		auto asset = hash_asset.second;
		auto path = asset->path.string();
		auto dir = asset->path.parent_path().string();

		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;

		std::string warn, err;
		assert(tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str(), dir.c_str()));

		asset->materials.clear();
		asset->constMaterials.clear();
		m_InitMaterials(asset, materials, dir);
	}
}

void MeshAsset::m_InitMesh(
	Asset* asset,
	const tinyobj::attrib_t& attrib,
	const std::vector<tinyobj::shape_t>& shapes)
{
	for (auto& shape : shapes) {
		m_InitShape(asset, attrib, shape);
	}
}

void MeshAsset::m_InitShape(
	Asset* asset,
	const tinyobj::attrib_t& attrib,
	const tinyobj::shape_t& shape)
{
	auto& verts = attrib.vertices;
	auto& colors = attrib.colors;
	auto& normals = attrib.normals;
	auto& texcoords = attrib.texcoords;

	std::vector<Mesh4::Vertex> verteces;
	std::vector<int> indeces;

	verteces.reserve(verts.size() / 3);
	indeces.reserve(shape.mesh.indices.size());

	for (int i = 0; i < shape.mesh.indices.size(); ++i) {
		Mesh4::Vertex vertex;

		auto& index = shape.mesh.indices[i];
		auto v = (size_t)index.vertex_index * 3;
		auto n = (size_t)index.normal_index * 3;
		auto t = (size_t)index.texcoord_index * 2;

		if (index.vertex_index >= 0) {
			vertex.position = { verts[v], verts[v + 1], verts[v + 2], 0.0f };
			vertex.color = { colors[v], colors[v + 1], colors[v + 2], 1.0f };
		}
		if (index.normal_index >= 0)
			vertex.normal = { normals[n], normals[n + 1], normals[n + 2], 1.0f };

		if (index.texcoord_index >= 0)
			vertex.uv = { texcoords[t], texcoords[t + 1] };

		verteces.push_back(vertex);
		indeces.push_back(i);
	}

	int matIndex = shape.mesh.material_ids[0];
	matIndex = matIndex >= 0 ? matIndex : 0;

	asset->mesh.AddShape(&verteces, &indeces, m_game->render(), matIndex);
}

void MeshAsset::m_InitMaterials(
	Asset* asset,
	const std::vector<tinyobj::material_t>& materials,
	const std::string& directory)
{
	auto* render = m_game->render();
	auto* images = m_game->imageAsset();
	const auto* shader = m_game->shaderAsset()->GetShader(m_game->shaderAsset()->GetShaderHash(L"data/engine/shaders/default.hlsl"));

	if (materials.empty()) {
		auto& mat = asset->materials.emplace_back();
		mat.name = "Defaulf";
		mat.shader = shader;

		const auto* image = images->Get(ImageAsset::RUNTIME_IMG_2X2_RGBA_1111);
		render->CreateTexture(image, mat.diffuse, false, false);
	}

	for (auto& tinyMat : materials) {
		auto& mat = asset->materials.emplace_back();
		mat.name = tinyMat.name;
		mat.shader = shader;

		mat.data.diffuse = tinyMat.diffuse[0];
		mat.data.ambient = tinyMat.ambient[0];
		mat.data.specular = tinyMat.specular[0];
		mat.data.shininess = tinyMat.shininess;

		auto& dc = tinyMat.diffuse;
		mat.data.diffuseColor = Vector3(dc[0], dc[1], dc[2]);

		if (tinyMat.diffuse_texname != "") {
			mat.diffuse.path = directory + "/" + tinyMat.diffuse_texname;

			if (!FileSystem::File::Exist(mat.diffuse.path)) {
				std::cout << "Warning! MeshAsset: Image not found (" << mat.diffuse.path << ") " << std::endl;
				mat.diffuse.path.clear();
			}
		}

		const auto* image = mat.diffuse.path.empty()
			? images->Get(ImageAsset::RUNTIME_IMG_2X2_RGBA_1111)
			: images->Get(mat.diffuse.path);

		render->CreateTexture(image, mat.diffuse, false, false);
	}

	asset->InitConstMaterials();
}

static void s_LoadMeshScales(std::ifstream& file, std::map<std::string, Vector3>* meshScales) {
	std::string data((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));
	auto jdata = nlohmann::json::parse(data);

	for (auto& jitem : jdata["meshes"].items()) {
		auto jscale = jitem.value()["scale"];
		Vector3 scale;
		scale.x = jscale["x"].get<float>();
		scale.y = jscale["y"].get<float>();
		scale.z = jscale["z"].get<float>();

		meshScales->insert({ jitem.key(), scale });
	}
}

static struct ObjectData {
	std::string obj;
	std::string name;
	Vector3 pos;
	Vector3 rot;
	Vector3 scale;
};

static struct SceneData {
	std::string sceneObj;
	std::vector<ObjectData> objects;
};

static void s_LoadSceneData(std::ifstream& file, SceneData* scene) {
	std::string data((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));
	auto jdata = nlohmann::json::parse(data);

	scene->sceneObj = jdata["staticMesh"].get<std::string>();

	for (auto& jitem : jdata["gameObjects"]) {
		auto jobject = jitem["gameObject"];
		auto& data = scene->objects.emplace_back();

		data.obj = jobject["obj"].get<std::string>();
		data.name = jobject["name"].get<std::string>();

		auto jpos = jobject["pos"];
		data.pos.x = jpos["x"].get<float>();
		data.pos.y = jpos["y"].get<float>();
		data.pos.z = jpos["z"].get<float>();

		auto jrot = jobject["rot"];
		data.rot.x = jrot["x"].get<float>();
		data.rot.y = jrot["y"].get<float>();
		data.rot.z = jrot["z"].get<float>();

		auto jscale = jobject["scale"];
		data.scale.x = jscale["x"].get<float>();
		data.scale.y = jscale["y"].get<float>();
		data.scale.z = jscale["z"].get<float>();
	}
}

void MeshAsset::LoadScene(fs::path levelDir, std::vector<GameObject*>* objects) {
	auto dir = levelDir.string();
	auto sceneJsonPath = levelDir.string() + "/scene.json";
	auto meshesJsonPath = levelDir.string() + "/meshes.json";

	std::ifstream fscene(sceneJsonPath);
	std::ifstream fmeshes(meshesJsonPath);

	if (!fscene)
		std::cout << "Can't load scene: file not open (" << sceneJsonPath << ")\n";

	if (!fmeshes)
		std::cout << "Can't load scene: file not open (" << meshesJsonPath << ")\n";

	if (!fscene || !fmeshes)
		return;

	std::map<std::string, Vector3> meshScales;
	SceneData scene;

	s_LoadMeshScales(fmeshes, &meshScales);
	s_LoadSceneData(fscene, &scene);

	auto sceneMeshPath = dir + "/" + scene.sceneObj;
	Load(sceneMeshPath);

	auto sceneObj = m_game->CreateGameObject("scene");
	sceneObj->AddComponent<MeshComponent>()->CreateMesh(sceneMeshPath);

	if (objects != nullptr)
		objects->push_back(sceneObj);

	for (auto& data : scene.objects) {
		auto meshPath = dir + "/" + data.obj;
		Load(meshPath);

		auto meshComp = m_game->CreateGameObject(data.name)->AddComponent<MeshComponent>();
		meshComp->CreateMesh(meshPath);

		meshComp->transform.localPosition(data.pos);
		meshComp->transform.localRotation(data.rot);
		meshComp->transform.localScale(data.scale / meshScales[data.name + ".obj"]);

		DirectX::BoundingBox box;
		auto mesh = m_GetMeshMutable(meshPath);

		for (int i = 0; i < mesh->shapeCount(); ++i) {
			auto shape = mesh->GetShape(i);

			auto p = (DirectX::XMFLOAT3*)&shape->verteces->position;
			auto count = shape->vertecesSize;

			DirectX::BoundingBox shapeBox;
			DirectX::BoundingBox::CreateFromPoints(shapeBox, count, p, sizeof(Mesh4::Vertex));
			DirectX::BoundingBox::CreateMerged(box, shapeBox, box);
		}
		auto hs = box.Extents;
		float radius = (hs.x + hs.y + hs.z) / 3;

		auto attachable = m_game->CreateGameObject("Attachable")->AddComponent<Attachable>();
		attachable->SetParent(meshComp);
		attachable->transform.localPosition(box.Center);
		attachable->boundRadius = radius;
		attachable->attachParent = true;
		attachable->showCenter = false;
		attachable->showBound = false;

		if (objects != nullptr) {
			objects->push_back(meshComp->gameObject());
			objects->push_back(attachable->gameObject());
		}
	}
}
