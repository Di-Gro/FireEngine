#pragma once

#include <map>
#include <unordered_map>
#include <string>
#include <vector>

#include "FileSystem.h"
#include "Mesh.h"
#include "Refs.h"
#include "CSBridge.h"
#include "Forms.h"

class Game;
class GameObject;
class Material;

namespace tinyobj {
	struct shape_t;
	struct attrib_t;
	struct material_t;
}

class MeshAsset {
public:
	static std::string defaultMaterialName;
	static std::string defaultShader;
	static std::string formBox;
	static std::string formBoxLined;
	static std::string formSphere;
	static std::string formSphereLined;

private:
	class Asset {
	public:
		fs::path path;

		Mesh4* mesh;
		//std::vector<Material> materials;
		std::vector<const Material*> staticMaterials;

		//void InitConstMaterials();

		Asset(size_t hash) {
			mesh = new Mesh4();
			mesh->f_assetHash = hash;
			mesh->f_ref = Refs::Create(mesh);
			mesh->f_cppRef = mesh->f_ref.id();
		}

		~Asset() {
			Refs::Remove(mesh->f_ref);
			delete mesh;
		}
	};

	enum class AssetType {
		Static,
		Dynamic,
	};

private:
	Game* m_game = nullptr;

	std::unordered_map<size_t, std::string> m_assetPaths;
	std::unordered_map<size_t, Asset*> m_assets;
	std::unordered_map<size_t, Material*> m_materials;
	std::unordered_map<size_t, Material*> m_dynamicMaterials;

public:
	MeshAsset() {}
	~MeshAsset();

	void Init(Game* game);
	void Start();
	void InitMono();

	void Load(fs::path path);
	void Load(size_t hash);
	void LoadScene(fs::path path, std::vector<GameObject*>* objects = nullptr);

	void ReloadMaterials();

	const Mesh4* GetMesh(fs::path path);
	const Mesh4* GetMesh(size_t hash);
	const std::vector<const Material*>* GetMaterials(fs::path path);
	const std::vector<const Material*>* GetMaterials(const Mesh4* mesh);

	size_t CreateHash(const fs::path& path);

	const Material* GetStaticMaterial(const std::string& name);
	const Material* GetStaticMaterial(const tinyobj::material_t& tinyMat, const std::string& directory);

	//Material* CreateMaterial(const std::string& name, const fs::path& shaderPath = MeshAsset::defaultShader);

	Material* CreateDynamicMaterial(const Material*);
	Material* CreateDynamicMaterial(const std::string& name, const fs::path& shaderPath = MeshAsset::defaultShader);
	void DeleteDynamicMaterial(Material*);


private:
	Asset* m_CreateMeshAsset(size_t hash, fs::path path);

	void m_InitDefaultMaterials();
	void m_InitDefaultStaticMeshes();
	void m_InitAssetFromForm(Forms4::Form& form, const fs::path& path);

	void m_Load(size_t hash, fs::path path);
	void m_InitMesh(Asset* asset, const tinyobj::attrib_t& attrib, const std::vector<tinyobj::shape_t>& shapes);
	void m_InitShape(Asset* asset, const tinyobj::attrib_t& attrib, const tinyobj::shape_t& shape);

	void m_InitMaterials(Asset* asset, const std::vector<tinyobj::material_t>& materials, const std::string& directory);

	Mesh4* m_GetMeshMutable(fs::path path);

	Material* m_LoadMaterial(size_t hash, const tinyobj::material_t& tinyMat, const std::string& dir);

	Material* m_NewMaterial();
	void m_DeleteMaterial(Material* mat);

};

FUNC(MeshAsset, CreateHash, size_t)(CppRef meshAssetRef, const char* fileName);
FUNC(MeshAsset, Load, void)(CppRef meshAssetRef, size_t assetHash);
FUNC(MeshAsset, GetMesh, CppRef)(CppRef meshAssetRef, size_t assetHash);

FUNC(MeshAsset, CreateDynamicMaterial, CppRef)(CppRef meshAssetRef, CppRef otherMaterialRef);
FUNC(MeshAsset, DeleteDynamicMaterial, void)(CppRef meshAssetRef, CppRef otherMaterialRef);
