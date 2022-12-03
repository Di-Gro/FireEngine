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
#include "Forms.h"

std::string MeshAsset::defaultMaterialName = "Defaulf";
std::string MeshAsset::defaultShader = "../../data/engine/shaders/default.hlsl";
std::string MeshAsset::formBox = "runtime:/form/Box";
std::string MeshAsset::formBoxLined = "runtime:/form/BoxLined";
std::string MeshAsset::formSphere = "runtime:/form/Sphere";
std::string MeshAsset::formSphereLined = "runtime:/form/SphereLined";



size_t MeshAsset::CreateHash(const fs::path& path) {
	auto hash = std::hash<std::string>()(path.string());

	m_assetPaths[hash] = path.string();

	return hash;
}

void MeshAsset::Init(Game* game) {
	m_game = game;
	InitMono();
}

void MeshAsset::Start() {
	m_InitDefaultMaterials();
	m_InitDefaultStaticMeshes();
}

void MeshAsset::InitMono() {
	auto type = m_game->mono()->GetType("Engine", "Game");
	auto method = mono::make_method_invoker<void(CppRef)>(type, "cpp_SetMeshAssetRef");

	auto ref = CppRefs::Create(this);
	method(CppRef::Create(ref.cppRef()));
}

MeshAsset::~MeshAsset() {
	for (auto& hash_asset : m_assets)
		delete hash_asset.second;

	for (auto it = m_materials.begin(); it != m_materials.end(); it++) {
		auto* material = it->second;
		m_DeleteMaterial(material);
	}

	for (auto it = m_dynamicMaterials.begin(); it != m_dynamicMaterials.end(); it++) {
		auto* material = it->second;
		m_DeleteMaterial(material);
	}
}

Material* MeshAsset::m_NewMaterial() {
	auto* material = new Material();
	material->f_ref = CppRefs::Create(material);
	material->f_cppRef = material->f_ref.cppRef();
	return material;
}

void MeshAsset::m_DeleteMaterial(Material* material) {
	CppRefs::Remove(material->f_ref);
	delete material;
}

void MeshAsset::Load(fs::path path) {
	m_Load(CreateHash(path), path);
}

void MeshAsset::Load(size_t hash) {
	auto path = m_assetPaths.at(hash);
	m_Load(hash, path);
}

void MeshAsset::m_Load(size_t hash, fs::path path) {
	if (m_assets.count(hash) > 0)
		return;

	//std::cout << "+: MeshAsset.m_Load(hash:" << hash << " path:" << path << ")" << std::endl;

	auto* asset = m_CreateMeshAsset(hash, path);
	m_assets.insert({ hash, asset });
}

const Mesh4* MeshAsset::GetMesh(fs::path path) {
	auto hash = CreateHash(path);

	m_Load(hash, path);
	return m_assets.at(hash)->mesh;
}

const Mesh4* MeshAsset::GetMesh(size_t hash) {
	auto path = m_assetPaths.at(hash);

	m_Load(hash, path);
	return m_assets.at(hash)->mesh;
}

Mesh4* MeshAsset::m_GetMeshMutable(fs::path path) {
	auto hash = CreateHash(path);

	m_Load(hash, path);
	return m_assets.at(hash)->mesh;
}

const std::vector<const Material*>* MeshAsset::GetMaterials(fs::path path) {
	auto hash = CreateHash(path);

	m_Load(hash, path);
	return &m_assets.at(hash)->staticMaterials;
}

const std::vector<const Material*>* MeshAsset::GetMaterials(const Mesh4* mesh) {
	if (mesh->f_assetHash == 0)
		return nullptr;

	auto path = m_assetPaths.at(mesh->f_assetHash);

	m_Load(mesh->f_assetHash, path);
	return &m_assets.at(mesh->f_assetHash)->staticMaterials;
}

MeshAsset::Asset* MeshAsset::m_CreateMeshAsset(size_t hash, fs::path path) {
	Asset* asset = nullptr;

	auto dir = path.parent_path().string();

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string warn, err;
	assert(tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.string().c_str(), dir.c_str()));

	asset = new Asset(hash);
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

		//asset->materials.clear();
		asset->staticMaterials.clear();
		m_InitMaterials(asset, materials, dir);
	}
}

Material* MeshAsset::CreateDynamicMaterial(const std::string& name, const fs::path& shaderPath) {
	auto* render = m_game->render();
	auto* images = m_game->imageAsset();
	auto* shaderAsset = m_game->shaderAsset();

	auto mat = m_NewMaterial();
	mat->isDynamic = true;
	mat->name(name);
	mat->shader = shaderAsset->GetShader(shaderAsset->GetShaderHash(shaderPath));

	const auto* image = m_game->imageAsset()->Get(ImageAsset::RUNTIME_IMG_2X2_RGBA_1111);
	m_game->render()->CreateTexture(image, mat->diffuse, false, false);

	m_dynamicMaterials.insert({ mat->cppRef(), mat });
	return mat;
}

Material* MeshAsset::CreateDynamicMaterial(const Material* other) {
	auto* render = m_game->render();
	auto* images = m_game->imageAsset();

	auto mat = m_NewMaterial();
	mat->isDynamic = true;
	mat->name(other->name());
	mat->shader = other->shader;
	mat->data = other->data;
	mat->diffuse.path = other->diffuse.path;

	const auto* image = mat->diffuse.path.empty()
		? images->Get(ImageAsset::RUNTIME_IMG_2X2_RGBA_1111)
		: images->Get(mat->diffuse.path);

	render->CreateTexture(image, mat->diffuse, false, false);

	m_dynamicMaterials.insert({ mat->cppRef(), mat });
	return mat;
}

void MeshAsset::DeleteDynamicMaterial(Material* mat) {
	if (m_dynamicMaterials.count(mat->cppRef()) == 0)
		return;

	m_dynamicMaterials.erase(mat->cppRef());
	m_DeleteMaterial(mat);
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

	asset->mesh->AddShape(&verteces, &indeces, m_game->render(), matIndex);
}

void MeshAsset::m_InitDefaultMaterials() {
	auto* render = m_game->render();
	auto* images = m_game->imageAsset();
	const auto* shader = m_game->shaderAsset()->GetShader(m_game->shaderAsset()->GetShaderHash(defaultShader));

	auto hash = std::hash<std::string>()(defaultMaterialName);

	if (m_materials.count(hash) == 0) {
		auto* mat = m_NewMaterial();
		m_materials.insert({ hash, mat });

		mat->name(defaultMaterialName);
		mat->shader = shader;

		const auto* image = images->Get(ImageAsset::RUNTIME_IMG_2X2_RGBA_1111);
		render->CreateTexture(image, mat->diffuse, false, false);
	}
}

void MeshAsset::m_InitDefaultStaticMeshes() {
	auto form1 = Forms4::Box({ 1, 1, 1 }, { 1, 1, 1, 1 });
	auto form2 = Forms4::BoxLined({ 1, 1, 1 }, { 1, 1, 1, 1 });
	auto form3 = Forms4::Sphere(0.5, 12, 12, { 1, 1, 1, 1 });
	auto form4 = Forms4::SphereLined(0.5, 12, 12, { 1, 1, 1, 1 });

	m_InitAssetFromForm(form1, MeshAsset::formBox);
	m_InitAssetFromForm(form2, MeshAsset::formBoxLined);
	m_InitAssetFromForm(form3, MeshAsset::formSphere);
	m_InitAssetFromForm(form4, MeshAsset::formSphereLined);
}

void MeshAsset::m_InitAssetFromForm(Forms4::Form& form, const fs::path& path) {
	auto hash = CreateHash(path);
	auto material = GetStaticMaterial(MeshAsset::defaultMaterialName);

	auto* asset = new Asset(hash);
	asset->path = path;
	asset->mesh->AddShape(&form.verteces, &form.indexes, m_game->render(), 0);
	asset->mesh->topology = form.topology;
	asset->staticMaterials.push_back(material);

	m_assets.insert({ hash, asset });
}

void MeshAsset::m_InitMaterials(
	Asset* asset,
	const std::vector<tinyobj::material_t>& materials,
	const std::string& directory)
{
	auto* render = m_game->render();
	auto* images = m_game->imageAsset();
	const auto* shader = m_game->shaderAsset()->GetShader(m_game->shaderAsset()->GetShaderHash(defaultShader));

	if (materials.empty()) {
		/// TODO: заменить на GetStaticMaterial(defaultMaterialName)

		auto hash = std::hash<std::string>()(defaultMaterialName);

		if (m_materials.count(hash) == 0) {
			auto* mat = m_NewMaterial();
			m_materials.insert({ hash, mat });

			mat->name(defaultMaterialName);
			mat->shader = shader;

			const auto* image = images->Get(ImageAsset::RUNTIME_IMG_2X2_RGBA_1111);
			render->CreateTexture(image, mat->diffuse, false, false);
		}
		asset->staticMaterials.push_back(m_materials.at(hash));
	}
	for (auto& tinyMat : materials) {
		auto mat = GetStaticMaterial(tinyMat, directory);
		asset->staticMaterials.push_back(mat);
	}
}

const Material* MeshAsset::GetStaticMaterial(const std::string& name) {
	auto hash = std::hash<std::string>()(name);

	if (m_materials.count(hash) > 0)
		return m_materials.at(hash);

	return nullptr;
}

const Material* MeshAsset::GetStaticMaterial(const tinyobj::material_t& tinyMat, const std::string& directory) {
	auto hash = std::hash<std::string>()(tinyMat.name);

	if (m_materials.count(hash) > 0)
		return m_materials.at(hash);

	m_LoadMaterial(hash, tinyMat, directory);
	return m_materials.at(hash);
}

Material* MeshAsset::m_LoadMaterial(
	size_t hash,
	const tinyobj::material_t& tinyMat,
	const std::string& directory)
{
	auto* render = m_game->render();
	auto* images = m_game->imageAsset();
	const auto* shader = m_game->shaderAsset()->GetShader(m_game->shaderAsset()->GetShaderHash(L"../../data/engine/shaders/default.hlsl"));

	auto material = m_NewMaterial();
	m_materials.insert({ hash, material });

	auto& mat = *material;
	mat.name(tinyMat.name);
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

	return material;
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

void MeshAsset::LoadScene(fs::path levelDir, std::vector<Actor*>* objects) {
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

	auto sceneObj = m_game->CreateActor("scene");
	sceneObj->AddComponent<MeshComponent>()->mesh(GetMesh(sceneMeshPath));

	if (objects != nullptr)
		objects->push_back(sceneObj);

	for (auto& data : scene.objects) {
		auto meshPath = dir + "/" + data.obj;
		Load(meshPath);

		auto meshComp = m_game->CreateActor(data.name)->AddComponent<MeshComponent>();
		meshComp->mesh(GetMesh(meshPath));

		meshComp->localPosition(data.pos);
		meshComp->localRotation(data.rot);
		meshComp->localScale(data.scale / meshScales[data.name + ".obj"]);

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

		//auto attachable = m_game->CreateActor("Attachable")->AddComponent<Attachable>();
		//attachable->actor()->parent(meshComp->actor());
		auto attachable = meshComp->actor()->AddComponent<Attachable>();

		//attachable->localPosition(box.Center);
		attachable->boundCenter = box.Center;

		attachable->boundRadius = radius;
		attachable->attachParent = true;
		attachable->showCenter = false;
		attachable->showBound = false;

		if (objects != nullptr) {
			objects->push_back(meshComp->actor());
			objects->push_back(attachable->actor());
		}
	}
}


DEF_FUNC(MeshAsset, CreateHash, size_t)(CppRef meshAssetRef, const char* fileName) {
	return CppRefs::ThrowPointer<MeshAsset>(meshAssetRef)->CreateHash(fileName);
}

FUNC(MeshAsset, Load, void)(CppRef meshAssetRef, size_t assetHash) {
	CppRefs::ThrowPointer<MeshAsset>(meshAssetRef)->Load(assetHash);
}

DEF_FUNC(MeshAsset, GetMesh, CppRef)(CppRef meshAssetRef, size_t assetHash) {
	return CppRefs::ThrowPointer<MeshAsset>(meshAssetRef)->GetMesh(assetHash)->f_cppRef;
}

DEF_FUNC(MeshAsset, CreateDynamicMaterial, CppRef)(CppRef meshAssetRef, CppRef otherMaterialRef) {
	auto material = CppRefs::ThrowPointer<Material>(otherMaterialRef);
	auto meshAsset = CppRefs::ThrowPointer<MeshAsset>(meshAssetRef);

	auto newMaterial = meshAsset->CreateDynamicMaterial(material);

	return newMaterial->cppRef();
}

DEF_FUNC(MeshAsset, DeleteDynamicMaterial, void)(CppRef meshAssetRef, CppRef otherMaterialRef) {
	auto material = CppRefs::ThrowPointer<Material>(otherMaterialRef);
	auto meshAsset = CppRefs::ThrowPointer<MeshAsset>(meshAssetRef);

	meshAsset->DeleteDynamicMaterial(material);
}