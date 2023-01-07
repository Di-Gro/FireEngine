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
#include "Scene.h"
#include "Render.h"
#include "ShaderAsset.h"
#include "ImageAsset.h"
#include "Assets.h"
#include "Actor.h"

#include "Texture.h"
#include "Material.h"

#include "MeshComponent.h"
#include "Attachable.h"
#include "Forms.h"
#include "Random.h"

std::string MeshAsset::materialDefault = "Defaulf";

std::string MeshAsset::formBox = "runtime:/form/Box";
std::string MeshAsset::formBoxLined = "runtime:/form/BoxLined";
std::string MeshAsset::formSphere = "runtime:/form/Sphere";
std::string MeshAsset::formSphereLined = "runtime:/form/SphereLined";
std::string MeshAsset::formCone = "../../data/engine/models/cone.obj";



int MeshAsset::CreateHash(const fs::path& path) {
	auto pathStr = path.string();
	auto hash = m_game->assets()->GetCsHash(pathStr);

	m_assetPaths[hash] = pathStr;

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
	auto ref = CppRefs::Create(this);
	m_game->callbacks().setMeshAssetRef(RefCpp(ref.cppRef()));
}

void MeshAsset::Destroy() {
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

Material* MeshAsset::m_NewMaterialAsset(const std::string& assetId) {
	auto assetIdHash = m_game->assets()->GetCsHash(assetId);
	auto cppRef = Material_PushAsset(CppRefs::GetRef(m_game), assetId.c_str(), assetIdHash);
	auto material = CppRefs::ThrowPointer<Material>(cppRef);

	material->Init(m_game->render());

	return material;
}

void MeshAsset::m_DeleteMaterialAsset(Material* material) {
	m_game->assets()->Pop(material->assetIdHash());
	delete material;
}

Texture* MeshAsset::m_NewTextureAsset(const std::string& assetId) {
	auto assetIdHash = m_game->assets()->GetCsHash(assetId);
	auto texCppRef = Texture_PushAsset(CppRefs::GetRef(m_game), assetId.c_str(), assetIdHash);

	return CppRefs::ThrowPointer<Texture>(texCppRef);
}

Mesh4* MeshAsset::m_NewMeshAsset(const std::string& assetId) {
	auto assetIdHash = m_game->assets()->GetCsHash(assetId);
	auto cppRef = Mesh4_PushAsset(CppRefs::GetRef(m_game), assetId.c_str(), assetIdHash);

	return CppRefs::ThrowPointer<Mesh4>(cppRef);
}

void MeshAsset::Load(fs::path path) {
	m_Load(CreateHash(path), path);
}

void MeshAsset::Load(int hash) {
	auto path = m_assetPaths.at(hash);
	m_Load(hash, path);
}

void MeshAsset::m_Load(int hash, fs::path path) {
	if (m_game->assets()->Contains(hash))
		return;

	m_CreateMeshAsset(hash, path);
}

const Mesh4* MeshAsset::GetMesh(fs::path path) {
	auto hash = CreateHash(path);

	if (!m_game->assets()->Contains(hash))
		m_Load(hash, path);

	return (Mesh4*)m_game->assets()->Get(hash);
}

const Mesh4* MeshAsset::GetMesh(int hash) {
	if (!m_game->assets()->Contains(hash)) {
		auto path = m_assetPaths.at(hash);
		m_Load(hash, path);
	}
	return (Mesh4*)m_game->assets()->Get(hash);
}

Mesh4* MeshAsset::m_GetMeshMutable(fs::path path) {
	auto hash = CreateHash(path);

	m_Load(hash, path);
	return(Mesh4*)m_game->assets()->Get(hash);
}

const std::vector<const Material*>* MeshAsset::GetMaterials(fs::path path) {
	auto hash = CreateHash(path);
	
	m_Load(hash, path);

	auto mesh = (Mesh4*)m_game->assets()->Get(hash);
	return &mesh->f_staticMaterials;
}

const std::vector<const Material*>* MeshAsset::GetMaterials(const Mesh4* mesh) {
	return &mesh->f_staticMaterials;
}

void MeshAsset::InitMesh(Mesh4* mesh, const fs::path& path) {
	auto dir = path.parent_path().string();

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string warn, err;
	assert(tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.string().c_str(), dir.c_str()));

	m_InitMesh(mesh, attrib, shapes);

	if (err != "")
		std::cout << err << std::endl;

	if (warn != "")
		std::cout << warn << std::endl;
}

Mesh4* MeshAsset::m_CreateMeshAsset(int hash, fs::path path) {
	auto dir = path.parent_path().string();	

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string warn, err;
	assert(tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.string().c_str(), dir.c_str()));

	auto assetId = path.string();
	auto mesh = m_NewMeshAsset(assetId);

	m_InitMesh(mesh, attrib, shapes);
	m_InitMaterials(mesh, materials, dir);

	if (err != "")
		std::cout << err << std::endl;

	if (warn != "")
		std::cout << warn << std::endl;

	return mesh;
}

void MeshAsset::ReloadMaterials() {
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

Material* MeshAsset::CreateDynamicMaterial(const std::string& name, const fs::path& shaderPath) {
	auto* render = m_game->render();
	auto* images = m_game->imageAsset();
	auto* shaderAsset = m_game->shaderAsset();

	auto matAssetId = name + std::to_string(Random().Int());
	auto mat = m_NewMaterialAsset(matAssetId);

	mat->isDynamic = true;
	mat->name(name);
	mat->shader = shaderAsset->GetShader(shaderAsset->GetShaderHash(shaderPath));

	const auto* image = m_game->imageAsset()->Get(ImageAsset::RUNTIME_IMG_2X2_RGBA_1111);

	auto texAssetId = name + "deffuseTex" + std::to_string(Random().Int());
	auto deffuseTex = m_NewTextureAsset(texAssetId);

	*deffuseTex = Texture::CreateFromImage(render, image);

	mat->textures.push_back(deffuseTex);
	mat->resources.emplace_back(ShaderResource::Create(deffuseTex));

	m_dynamicMaterials.insert({ CppRefs::GetRef((void*)mat), mat });
	return mat;
}

Material* MeshAsset::CreateDynamicMaterial(const Material* other) {
	auto* render = m_game->render();
	auto* images = m_game->imageAsset();

	auto mat = m_NewMaterialAsset(m_game->assets()->CreateAssetId());
	mat->priority = other->priority;
	mat->isDynamic = true;
	mat->name(other->name());
	mat->shader = other->shader;
	mat->data = other->data;
	
	for (int i = 0; i < other->textures.size(); i++) {
		auto& otherTex = other->textures[i];

		const auto* image = otherTex->image;
		assert(image != nullptr);

			//otherTex->name.empty()
			//? images->Get(ImageAsset::RUNTIME_IMG_2X2_RGBA_1111)
			//: images->Get(otherTex->name);

		auto assetId = otherTex->name + "deffuseTex" + std::to_string(Random().Int());
		auto thisTex = m_NewTextureAsset(assetId);

		*thisTex = Texture::CreateFromImage(render, image);

		mat->textures.push_back(thisTex);
		mat->resources.emplace_back(ShaderResource::Create(thisTex));

		thisTex->name = otherTex->name;
	}

	m_dynamicMaterials.insert({ CppRefs::GetRef((void*)mat), mat});
	return mat;
}

void MeshAsset::DeleteDynamicMaterial(Material* mat) {
	auto matRef = CppRefs::GetRef((void*)mat);
	if (m_dynamicMaterials.count(matRef) == 0)
		return;

	m_game->DeleteMaterialFromAllScenes(mat);

	m_dynamicMaterials.erase(matRef);
	m_DeleteMaterialAsset(mat);
}

void MeshAsset::m_InitMesh(
	Mesh4* mesh,
	const tinyobj::attrib_t& attrib,
	const std::vector<tinyobj::shape_t>& shapes)
{
	for (auto& shape : shapes) {
		m_InitShape(mesh, attrib, shape);
	}
}

void MeshAsset::m_InitShape(
	Mesh4* mesh,
	const tinyobj::attrib_t& attrib,
	const tinyobj::shape_t& shape)
{
	auto& verts = attrib.vertices;

	std::vector<Mesh4::Vertex> verteces;
	std::vector<int> indeces;

	verteces.reserve(verts.size() / 3);
	indeces.reserve(shape.mesh.indices.size());

	int faceSize = 3;
	for (int i = 0; i < shape.mesh.indices.size(); i += faceSize) {
		auto vertex1 = m_ReadVertex(attrib, shape.mesh.indices[i + 0]);
		auto vertex2 = m_ReadVertex(attrib, shape.mesh.indices[i + 1]);
		auto vertex3 = m_ReadVertex(attrib, shape.mesh.indices[i + 2]);

		verteces.insert(verteces.end(), { vertex1 , vertex2, vertex3 });
		indeces.insert(indeces.end(), { i + 2, i + 1, i + 0 });
	}

	int matIndex = shape.mesh.material_ids[0];
	matIndex = matIndex >= 0 ? matIndex : 0;

	mesh->AddShape(&verteces, &indeces, m_game->render(), matIndex);
}

Mesh4::Vertex MeshAsset::m_ReadVertex(const tinyobj::attrib_t& attrib, const tinyobj::index_t& index) {
	auto& verts = attrib.vertices;
	auto& colors = attrib.colors;
	auto& normals = attrib.normals;
	auto& texcoords = attrib.texcoords;

	Mesh4::Vertex vertex;

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

	return vertex;
}

void MeshAsset::m_InitDefaultMaterials() {
	auto* render = m_game->render();
	auto* images = m_game->imageAsset();
	const auto* shader = m_game->shaderAsset()->GetShader(m_game->shaderAsset()->GetShaderHash(Assets::ShaderDefault));

	auto hash = std::hash<std::string>()(materialDefault);

	if (m_materials.count(hash) == 0) {
		auto* mat = m_NewMaterialAsset(m_game->assets()->CreateAssetId());
		m_materials.insert({ hash, mat });

		mat->name(materialDefault);
		mat->shader = shader;

		const auto* image = images->Get(ImageAsset::RUNTIME_IMG_2X2_RGBA_1111);

		auto assetId = materialDefault + "deffuseTex";
		auto deffuseTex = m_NewTextureAsset(assetId);

		*deffuseTex = Texture::CreateFromImage(render, image);

		mat->textures.push_back(deffuseTex);
		mat->resources.emplace_back(ShaderResource::Create(deffuseTex));
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
	auto material = GetStaticMaterial(MeshAsset::materialDefault);

	auto assetId = path.string();
	auto mesh = m_NewMeshAsset(assetId);

	mesh->AddShape(&form.verteces, &form.indexes, m_game->render(), 0);
	mesh->topology = form.topology;
	mesh->f_staticMaterials.push_back(material);
}

void MeshAsset::m_InitMaterials(
	Mesh4* mesh,
	const std::vector<tinyobj::material_t>& materials,
	const std::string& directory)
{
	auto* render = m_game->render();
	auto* images = m_game->imageAsset();
	const auto* shader = m_game->shaderAsset()->GetShader(m_game->shaderAsset()->GetShaderHash(Assets::ShaderDefault));

	if (materials.empty()) {
		auto* mat = GetStaticMaterial(MeshAsset::materialDefault);
		mesh->f_staticMaterials.push_back(mat);
	}
	for (auto& tinyMat : materials) {
		auto mat = GetStaticMaterial(tinyMat, directory);
		mesh->f_staticMaterials.push_back(mat);
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
	int hash,
	const tinyobj::material_t& tinyMat,
	const std::string& directory)
{
	auto* render = m_game->render();
	auto* images = m_game->imageAsset();
	const auto* shader = m_game->shaderAsset()->GetShader(m_game->shaderAsset()->GetShaderHash(Assets::ShaderDefault));

	auto material = m_NewMaterialAsset(m_game->assets()->CreateAssetId());
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

	auto assetId = directory + "/" + tinyMat.diffuse_texname;
	auto deffuseTex = m_NewTextureAsset(assetId);
	mat.textures.push_back(deffuseTex);

	if (tinyMat.diffuse_texname != "") {
		deffuseTex->name = directory + "/" + tinyMat.diffuse_texname;

		if (!FileSystem::File::Exist(deffuseTex->name)) {
			std::cout << "Warning! MeshAsset: Image not found (" << deffuseTex->name << ") " << std::endl;
			deffuseTex->name.clear();
		}
	}

	const auto* image = deffuseTex->name.empty()
		? images->Get(ImageAsset::RUNTIME_IMG_2X2_RGBA_1111)
		: images->Get(deffuseTex->name);

	std::string name = deffuseTex->name;
	*deffuseTex = Texture::CreateFromImage(render, image);
	deffuseTex->name = name;

	mat.resources.emplace_back(ShaderResource::Create(deffuseTex));

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

	auto sceneObj = m_game->currentScene()->CreateActor("scene");
	sceneObj->AddComponent<MeshComponent>()->mesh(GetMesh(sceneMeshPath));

	if (objects != nullptr)
		objects->push_back(sceneObj);

	for (auto& data : scene.objects) {
		auto meshPath = dir + "/" + data.obj;
		Load(meshPath);

		auto meshComp = m_game->currentScene()->CreateActor(data.name)->AddComponent<MeshComponent>();
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

FUNC(MeshAsset, Load, void)(CppRef meshAssetRef, int assetHash) {
	CppRefs::ThrowPointer<MeshAsset>(meshAssetRef)->Load(assetHash);
}

DEF_FUNC(MeshAsset, GetMesh, CppRef)(CppRef meshAssetRef, int assetHash) {
	auto mesh = CppRefs::ThrowPointer<MeshAsset>(meshAssetRef)->GetMesh(assetHash);
	return CppRefs::GetRef((void*)mesh);
}

DEF_FUNC(MeshAsset, CreateDynamicMaterial, CppRef)(CppRef meshAssetRef, CppRef otherMaterialRef) {
	auto material = CppRefs::ThrowPointer<Material>(otherMaterialRef);
	auto meshAsset = CppRefs::ThrowPointer<MeshAsset>(meshAssetRef);

	auto newMaterial = meshAsset->CreateDynamicMaterial(material);

	return CppRefs::GetRef(newMaterial);
}

DEF_FUNC(MeshAsset, DeleteDynamicMaterial, void)(CppRef meshAssetRef, CppRef otherMaterialRef) {
	auto material = CppRefs::ThrowPointer<Material>(otherMaterialRef);
	auto meshAsset = CppRefs::ThrowPointer<MeshAsset>(meshAssetRef);

	meshAsset->DeleteDynamicMaterial(material);
}