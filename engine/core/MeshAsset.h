#pragma once

#include <map>
#include <unordered_map>
#include <string>
#include <vector>

#include "Mesh.h"
#include "Refs.h"
#include "CSBridge.h"
#include "Forms.h"
#include "Assets.h"

class Game;
class Actor;
class Material;
class Texture;

namespace tinyobj {
	struct shape_t;
	struct attrib_t;
	struct material_t;
	struct index_t;
}

class MeshAsset {
public:
	static std::string materialDefault;

	static std::string formBox;
	static std::string formBoxLined;
	static std::string formSphere;
	static std::string formSphereLined;
	static std::string formCone;
	//static std::string formCapsule;

private:
	//class Asset {
	//public:
	//	//fs::path path;

	//	Mesh4* mesh;
	//	//std::vector<const Material*> staticMaterials;

	//	Asset(int hash) {
	//		mesh = new Mesh4();
	//		mesh->f_assetHash = hash;
	//		mesh->f_ref = CppRefs::Create(mesh);
	//		mesh->f_cppRef = mesh->f_ref.cppRef();
	//	}

	//	~Asset() {
	//		CppRefs::Remove(mesh->f_ref);
	//		delete mesh;
	//	}
	//};

	enum class AssetType {
		Static,
		Dynamic,
	};

private:
	Game* m_game = nullptr;

	std::unordered_map<int, std::string> m_assetPaths;
	//std::unordered_map<int, Asset*> m_assets;
	std::unordered_map<int, Material*> m_materials;
	std::unordered_map<int, Material*> m_dynamicMaterials;

public:
	MeshAsset() {}

	void Init(Game* game);
	void InitMono();
	void Start();
	void Destroy();

	void Load(fs::path path);
	void Load(int hash);

	void ReloadMaterials();

	const Mesh4* GetMesh(fs::path path);
	const Mesh4* GetMesh(int hash);
	const std::vector<const Material*>* GetMaterials(fs::path path);
	const std::vector<const Material*>* GetMaterials(const Mesh4* mesh);

	int CreateHash(const fs::path& path);

	const Material* GetStaticMaterial(const std::string& name);
	const Material* GetStaticMaterial(const tinyobj::material_t& tinyMat, const std::string& directory);

	Material* CreateDynamicMaterial(const Material*);
	Material* CreateDynamicMaterial(const std::string& name, const fs::path& shaderPath = Assets::ShaderDefault);
	void DeleteDynamicMaterial(Material*);

	void InitMesh(Mesh4* mesh, const fs::path& path);

private:
	Mesh4* m_CreateMeshAsset(int hash, fs::path path);

	void m_InitDefaultMaterials();
	void m_InitDefaultStaticMeshes();
	void m_InitAssetFromForm(Forms4::Form& form, const fs::path& path);

	void m_Load(int hash, fs::path path);
	void m_InitMesh(Mesh4* mesh, const tinyobj::attrib_t& attrib, const std::vector<tinyobj::shape_t>& shapes);
	void m_InitShape(Mesh4* mesh, const tinyobj::attrib_t& attrib, const tinyobj::shape_t& shape);

	Mesh4::Vertex m_ReadVertex(const tinyobj::attrib_t& attrib, const tinyobj::index_t& index);

	void m_InitMaterials(Mesh4* mesh, const std::vector<tinyobj::material_t>& materials, const std::string& directory);

	Material* m_LoadMaterial(int hash, const tinyobj::material_t& tinyMat, const std::string& dir);

	Material* m_NewMaterialAsset(const std::string& assetId);
	Texture* m_NewTextureAsset(const std::string& assetId);
	Mesh4* m_NewMeshAsset(const std::string& assetId);

	void m_DeleteMaterialAsset(Material* mat);
};

FUNC(MeshAsset, CreateDynamicMaterial, CppRef)(CppRef meshAssetRef, CppRef otherMaterialRef);
FUNC(MeshAsset, DeleteDynamicMaterial, void)(CppRef meshAssetRef, CppRef otherMaterialRef);
