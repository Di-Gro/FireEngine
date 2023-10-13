#pragma once

#include <map>
#include <unordered_map>
#include <string>
#include <vector>

#include "MeshAsset.h"
#include "MeshResource.h"
#include "Refs.h"
#include "CSBridge.h"
#include "Forms.h"
#include "Assets.h"
#include "Vertex.h"

class Game;
class Actor;
class MaterialResource;
class TextureAsset;
class MaterialAsset;

namespace tinyobj {
	struct shape_t;
	struct attrib_t;
	struct material_t;
	struct index_t;
}

class MeshAssets {
public:
	static std::string materialDefault;

	static std::string formBox;
	static std::string formBoxLined;
	static std::string formSphere;
	static std::string formSphereLined;
	static std::string formCone;

private:
	enum class AssetType {
		Static,
		Dynamic,
	};

private:
	Game* m_game = nullptr;

	std::unordered_map<int, std::string> m_assetPaths;
	std::unordered_map<int, MaterialAsset*> m_materials;
	std::unordered_map<int, MaterialAsset*> m_dynamicMaterials;

public:
	MeshAssets() {}

	void Init(Game* game);
	void InitMono();
	void Start();
	void Destroy();

	void Load(const fs::path& path);
	void Load(int hash);

	void ReloadMaterials();

	const MeshAsset* GetMesh(const fs::path& path);
	const MeshAsset* GetMesh(int hash);
	const std::vector<const MaterialAsset*>* GetMaterials(const fs::path& path);
	const std::vector<const MaterialAsset*>* GetMaterials(const MeshAsset* mesh);

	int CreateHash(const fs::path& path);

	const MaterialAsset* GetStaticMaterial(const std::string& name);
	const MaterialAsset* GetStaticMaterial(const tinyobj::material_t& tinyMat, const std::string& directory);

	MaterialAsset* CreateDynamicMaterial(const MaterialAsset*);
	MaterialAsset* CreateDynamicMaterial(const std::string& name, const fs::path& shaderPath = Assets::ShaderDefault);
	void DeleteDynamicMaterial(MaterialAsset*);

private:
	MeshAsset* m_CreateMeshAsset(int hash, const fs::path& path);

	void m_InitDefaultMaterials();
	void m_InitDefaultStaticMeshes();
	void m_InitAssetFromForm(Forms4::Form& form, const fs::path& path);

	void m_Load(int hash, const fs::path& path);
	void m_InitMaterials(MeshAsset* mesh, const std::vector<tinyobj::material_t>& materials, const std::string& directory);

	MaterialAsset* m_LoadMaterial(int hash, const tinyobj::material_t& tinyMat, const std::string& dir);
	
	MaterialAsset* m_NewMaterialAsset(const std::string& assetId);
	TextureAsset* m_NewTextureAsset(const std::string& assetId);
	MeshAsset* m_NewMeshAsset(const std::string& assetId);

	void m_DeleteMaterialAsset(MaterialAsset* mat);
};

FUNC(MeshAssets, CreateDynamicMaterial, CppRef)(CppRef meshAssetRef, CppRef otherMaterialRef);
FUNC(MeshAssets, DeleteDynamicMaterial, void)(CppRef meshAssetRef, CppRef otherMaterialRef);
