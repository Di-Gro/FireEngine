#include "ObjLoader.h"

#include <unordered_set>

#include "tiny_obj_loader.h"

#include "Game.h"
#include "Assets.h"
#include "Render.h"
#include "AssetStore.h"

#include "ShaderAsset.h"
#include "MaterialAsset.h"
#include "MeshAsset.h"
#include "TextureAsset.h"
#include "ImageAsset.h"

MeshAsset* ObjLoader::LoadMeshWithMaterials(const fs::path& path) {
	auto* assets = m_game->assets();
	auto* store = m_game->assetStore();

	auto dir = path.parent_path().string();
	auto filename = path.filename().string();

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> tinyMaterials;

	std::string warn, err;
	bool res = tinyobj::LoadObj(&attrib, &shapes, &tinyMaterials, &warn, &err, path.string().c_str(), dir.c_str());
	assert(res);
	
	auto assetName = "obj/" + filename;
	auto assetId = store->CreateRuntimeAssetId(assetName);
	auto mesh = new MeshAsset();

	store->AddRuntimeAssetId(assetId);
	assets->Push(assetId, mesh);

	auto materials = m_LoadMaterials(tinyMaterials, dir, assetName);

	mesh->resource = MeshResource::CreateFromObj(m_game->render(), path);
	mesh->f_staticMaterials = materials;

	if (err != "")
		std::cout << err << std::endl;

	//if (warn != "")
	//	std::cout << warn << std::endl;

	return mesh;
}

std::vector<const MaterialAsset*> ObjLoader::m_LoadMaterials(
	const std::vector<tinyobj::material_t>& tinyMaterials,
	const std::string& directory,
	const std::string& assetName)
{
	auto* render = m_game->render();
	auto* assets = m_game->assets();

	const auto* shader = m_game->shaderAsset()->GetShader(m_game->shaderAsset()->GetShaderHash(Assets::ShaderDefault));

	std::vector<const MaterialAsset*> materials;
	std::unordered_set<size_t> nameHashes;

	if (tinyMaterials.empty()) {
		auto* material = assets->GetStatic<MaterialAsset>(Assets::DefaultMaterial);
		materials.push_back(material);

		return materials;
	}
	materials.reserve(tinyMaterials.size());

	for (auto& tinyMat : tinyMaterials) {
		auto nameHash = std::hash<std::string>()(tinyMat.name);
		if (nameHashes.contains(nameHash))
			continue;

		nameHashes.insert(nameHash);

		auto material = m_LoadMaterial(tinyMat, directory, assetName);
		materials.push_back(material);
	}
	return materials;
}

MaterialAsset* ObjLoader::m_LoadMaterial(
	const tinyobj::material_t& tinyMat,
	const std::string& directory,
	const std::string& assetName)
{
	auto* render = m_game->render();
	auto* assets = m_game->assets();
	auto* store = m_game->assetStore();

	const auto* shader = m_game->shaderAsset()->GetShader(m_game->shaderAsset()->GetShaderHash(Assets::ShaderDefault));

	auto matAssetId = store->CreateRuntimeAssetId(assetName + "/" + tinyMat.name);
	auto* mat = new MaterialAsset();

	store->AddRuntimeAssetId(matAssetId);
	assets->Push(matAssetId, mat);

	mat->name(tinyMat.name);
	mat->resource.shader = shader;

	mat->resource.data.diffuse = tinyMat.diffuse[0];
	mat->resource.data.ambient = tinyMat.ambient[0];
	mat->resource.data.specular = tinyMat.specular[0];
	mat->resource.data.shininess = tinyMat.shininess;

	auto& dc = tinyMat.diffuse;
	mat->resource.data.diffuseColor = Vector3(dc[0], dc[1], dc[2]);

	auto texAssetId = store->CreateRuntimeAssetId(assetName + "/" + "deffuseTex");
	auto deffuseTex = new TextureAsset();

	store->AddRuntimeAssetId(texAssetId);
	assets->Push(texAssetId, deffuseTex);

	mat->textures.push_back(deffuseTex);

	if (tinyMat.diffuse_texname != "") {
		deffuseTex->name = directory + "/" + tinyMat.diffuse_texname;

		if (!fs::exists(deffuseTex->name)) {
			std::cout << "Warning! ObjLoader: Image not found (" << deffuseTex->name << ") " << std::endl;
			deffuseTex->name.clear();
		}
	}

	auto imagePath = deffuseTex->name.empty() ? Assets::Img2x2rgba1111 : deffuseTex->name;
	auto* image = assets->GetDinamic<ImageAsset>(deffuseTex->name);

	if (image == nullptr) {
		auto imgAssetId = store->CreateRuntimeAssetId(assetName + "/" + tinyMat.diffuse_texname);
		image = new ImageAsset();

		store->AddRuntimeAssetId(imgAssetId);
		assets->Push(imgAssetId, image);

		image->resource = ImageResource::CreateFromFile(imagePath);
	}

	auto name = deffuseTex->name;
	deffuseTex->resource = TextureResource::CreateFromImage(render, &image->resource);
	deffuseTex->name = name;

	mat->resource.textures.emplace_back(ShaderResource::Create(&deffuseTex->resource));

	return mat;
}
