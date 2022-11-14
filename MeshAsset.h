#pragma once

#include <map>
#include <string>
#include <vector>

#include "FileSystem.h"
#include "Mesh.h"

class Game;
class GameObject;
class Material;

namespace tinyobj {
	struct shape_t;
	struct attrib_t;
	struct material_t;
}

class MeshAsset {
private:
	class Asset {
	public:
		fs::path path;

		Mesh4 mesh;
		std::vector<Material> materials;
		std::vector<const Material*> constMaterials;

		void InitConstMaterials();
	};

private:
	Game* m_game = nullptr;

	std::map<unsigned int, Asset*> m_assets;

public:
	MeshAsset() {}
	~MeshAsset();

	void Init(Game* game) { m_game = game; }

	void Load(fs::path path);
	void LoadScene(fs::path path, std::vector<GameObject*>* objects = nullptr);

	void ReloadMaterials();

	const Mesh4* GetMesh(fs::path path);
	const std::vector<const Material*>* GetMaterials(fs::path path);

private:

	Asset* m_CreateMeshAsset(fs::path path);

	void m_Load(unsigned int hash, fs::path path);
	void m_InitMesh(Asset* asset, const tinyobj::attrib_t& attrib, const std::vector<tinyobj::shape_t>& shapes);
	void m_InitShape(Asset* asset, const tinyobj::attrib_t& attrib, const tinyobj::shape_t& shape);

	void m_InitMaterials(Asset* asset, const std::vector<tinyobj::material_t>& materials, const std::string& directory);

	Mesh4* m_GetMeshMutable(fs::path path);

public:

	static unsigned int GetHash(fs::path path);

};

