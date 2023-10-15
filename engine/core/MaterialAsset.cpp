#include "MaterialAsset.h"
#include "Render.h"
#include "Game.h"
#include "Assets.h"
#include "AssetStore.h"
#include "ShaderAsset.h"
#include "TextureAsset.h"
#include "ImageAsset.h"

const MaterialAsset* MaterialAsset::Default = nullptr;

std::unordered_set<size_t> MaterialAsset::m_dynamic;

void MaterialAsset::Release() {
	resource.Release();
}

MaterialAsset* MaterialAsset::CreateDynamic(Game* game, const std::string& name, const fs::path& shaderPath) {
	auto* render = game->render();
	auto* shaderAsset = game->shaderAsset();
	auto* assets = game->assets();
	auto* store = game->assetStore();

	auto assetId = store->CreateRuntimeAssetId(name);
	auto mat = new MaterialAsset();

	assets->Push(assetId, mat);
	m_dynamic.insert(CppRefs::GetRef(mat));

	mat->resource.Init(render);
	mat->isDynamic = true;
	mat->name(name);
	mat->resource.shader = shaderAsset->GetShader(shaderAsset->GetShaderHash(shaderPath));

	const auto* image = assets->GetStatic<ImageAsset>(Assets::Img2x2rgba1111);

	auto texAssetId = store->CreateRuntimeAssetId(name + "/deffuseTex");
	auto* deffuseTex = new TextureAsset();

	assets->Push(assetId, deffuseTex);
	m_dynamic.insert(CppRefs::GetRef(deffuseTex));

	deffuseTex->resource = TextureResource::CreateFromImage(render, &image->resource);

	mat->textures.push_back(deffuseTex);
	mat->resource.textures.emplace_back(ShaderResource::Create(&deffuseTex->resource));
	
	return mat;
}

MaterialAsset* MaterialAsset::CreateDynamic(Game* game, const MaterialAsset* otherRes) {
	auto* render = game->render();
	auto* assets = game->assets();
	auto* store = game->assetStore();

	auto assetId = store->CreateRuntimeAssetId();
	auto mat = new MaterialAsset();

	assets->Push(assetId, mat);
	m_dynamic.insert(CppRefs::GetRef(mat));

	mat->resource.Init(render);

	mat->resource.priority = otherRes->resource.priority;
	mat->isDynamic = true;
	mat->name(otherRes->name());
	mat->resource.shader = otherRes->resource.shader;
	mat->resource.data = otherRes->resource.data;

	for (int i = 0; i < otherRes->textures.size(); i++) {
		auto* otherTex = otherRes->textures[i];

		const auto* image = otherTex->image;
		assert(image != nullptr);

		auto texAssetId = store->CreateRuntimeAssetId(assetId + "/deffuseTex");
		auto thisTex = new TextureAsset();

		assets->Push(texAssetId, thisTex);
		m_dynamic.insert(CppRefs::GetRef(thisTex));

		thisTex->resource = TextureResource::CreateFromImage(render, &image->resource);

		mat->textures.push_back(thisTex);
		mat->resource.textures.emplace_back(ShaderResource::Create(&thisTex->resource));

		thisTex->name = otherTex->name;
	}
	return mat;
}

void MaterialAsset::DeleteDinamic(Game* game, MaterialAsset* material) {
	auto* assets = game->assets();

	auto matRef = CppRefs::GetRef(material);
	if (!m_dynamic.contains(matRef))
		return;

	game->DeleteMaterialFromAllScenes(&material->resource);

	for (int i = 0; i < material->textures.size(); i++) {
		auto* staticTex = material->textures[i];
		auto texRef = CppRefs::GetRef(staticTex);

		if (m_dynamic.contains(texRef)) {
			auto* dynamicTex = (TextureAsset*)staticTex;
			
			m_dynamic.erase(texRef);
			assets->Pop(dynamicTex->assetIdHash());
			dynamicTex->Release();
			delete dynamicTex;
		}
	}
	m_dynamic.erase(matRef);
	assets->Pop(material->assetIdHash());
	material->Release();
	delete material;
}

DEF_PUSH_ASSET(MaterialAsset);

DEF_PROP_GETSET_F(MaterialAsset, Vector3, diffuseColor, resource.data.diffuseColor)
DEF_PROP_GETSET_F(MaterialAsset, float, diffuse, resource.data.diffuse)
DEF_PROP_GETSET_F(MaterialAsset, float, ambient, resource.data.ambient)
DEF_PROP_GETSET_F(MaterialAsset, float, specular, resource.data.specular)
DEF_PROP_GETSET_F(MaterialAsset, float, shininess, resource.data.shininess)
DEF_PROP_GETSET_F(MaterialAsset, CullMode, cullMode, resource.cullMode)
DEF_PROP_GETSET_F(MaterialAsset, FillMode, fillMode, resource.fillMode)
DEF_PROP_GETSET_F(MaterialAsset, size_t, priority, resource.priority)

DEF_FUNC(MaterialAsset, name_set, void)(CppRef matRef, const char* name) {
	auto material = CppRefs::ThrowPointer<MaterialAsset>(matRef);
	material->name(name);
}

DEF_FUNC(MaterialAsset, name_length, size_t)(CppRef matRef) {
	auto material = CppRefs::ThrowPointer<MaterialAsset>(matRef);
	return material->name().size();
}

DEF_FUNC(MaterialAsset, name_get, void)(CppRef matRef, char* buf) {
	auto material = CppRefs::ThrowPointer<MaterialAsset>(matRef);
	auto name = material->name();

	for (int i = 0; i < name.size(); i++)
		buf[i] = name[i];
}

DEF_FUNC(MaterialAsset, shader_set, void)(CppRef gameRef, CppRef matRef, const char* name) {
	auto game = CppRefs::ThrowPointer<Game>(gameRef);
	auto material = CppRefs::ThrowPointer<MaterialAsset>(matRef);
	auto hash = game->shaderAsset()->GetShaderHash(name);

	bool hasShader = game->shaderAsset()->HasShader(hash);
	if (!hasShader)
		hasShader = game->shaderAsset()->TryCompileShader(name);

	if (hasShader)
		material->resource.shader = game->shaderAsset()->GetShader(hash);
	else {
		std::string str = "Missing Shader File: ";
		str += name;
		throw std::exception(str.c_str());
	}
}

DEF_FUNC(MaterialAsset, shader_length, size_t)(CppRef matRef) {
	auto material = CppRefs::ThrowPointer<MaterialAsset>(matRef);
	if (material->resource.shader == nullptr)
		return 0;
	return material->resource.shader->path.string().size();
}

DEF_FUNC(MaterialAsset, shader_get, void)(CppRef matRef, char* buf) {
	auto material = CppRefs::ThrowPointer<MaterialAsset>(matRef);
	auto name = material->resource.shader->path.string();

	for (int i = 0; i < name.size(); i++)
		buf[i] = name[i];
}

DEF_FUNC(MaterialAsset, isDynamic_get, bool)(CppRef matRef) {
	if (matRef == 0)
		return false;

	auto material = CppRefs::ThrowPointer<MaterialAsset>(matRef);
	return material->isDynamic;
}

DEF_FUNC(MaterialAsset, Init, void)(CppRef gameRef, CppRef matRef) {
	auto* game = CppRefs::ThrowPointer<Game>(gameRef);
	auto* material = CppRefs::ThrowPointer<MaterialAsset>(matRef);

	material->resource.Init(game->render());
}

DEF_FUNC(MaterialAsset, textures_set, void)(CppRef matRef, size_t* cppRefs, int count) {
	auto* material = CppRefs::ThrowPointer<MaterialAsset>(matRef);

	/// TODO: Удалить старые текстуры. 
	/// ----: Нужен Release для ShaderResource.
	material->textures.clear();
	material->resource.textures.clear();

	if (count == 0) {
		auto* texture = TextureAsset::Default;

		material->textures.push_back(texture);
		material->resource.textures.emplace_back(ShaderResource::Create(&texture->resource));
		return;
	}
	auto ptr = cppRefs;
	for (int i = 0; i < count; i++, ptr++) {
		auto* texture = TextureAsset::Default;

		if (*ptr != 0) {
			auto cppRef = RefCpp(*ptr);
			texture = CppRefs::ThrowPointer<TextureAsset>(cppRef);
		}
		material->textures.push_back(texture);
		material->resource.textures.emplace_back(ShaderResource::Create(&texture->resource));
	}
}

DEF_FUNC(MaterialAsset, CreateDynamicMaterial, CppRef)(CppRef gameRef, CppRef otherMaterialRef) {
	auto game = CppRefs::ThrowPointer<Game>(gameRef);
	auto material = CppRefs::ThrowPointer<MaterialAsset>(otherMaterialRef);

	auto newMaterial = MaterialAsset::CreateDynamic(game, material);

	return CppRefs::GetRef(newMaterial);
}

DEF_FUNC(MaterialAsset, DeleteDynamicMaterial, void)(CppRef gameRef, CppRef otherMaterialRef) {
	auto game = CppRefs::ThrowPointer<Game>(gameRef);
	auto material = CppRefs::ThrowPointer<MaterialAsset>(otherMaterialRef);

	MaterialAsset::DeleteDinamic(game, material);
}
