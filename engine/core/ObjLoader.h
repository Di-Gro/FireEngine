#pragma once
#include <string>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

class Game;
class MeshAsset;
class MaterialAsset;

namespace tinyobj {
	struct material_t;
}

class ObjLoader {
private:
	Game* m_game;

public:
	ObjLoader(Game* game) { 
		m_game = game; 
	}

	MeshAsset* LoadMeshWithMaterials(const fs::path& path);

private:
	MaterialAsset* m_LoadMaterial(const tinyobj::material_t& tinyMat, const std::string& dir, const std::string& assetName);
	std::vector<const MaterialAsset*> m_LoadMaterials(const std::vector<tinyobj::material_t>& materials, const std::string& directory, const std::string& assetName);
};