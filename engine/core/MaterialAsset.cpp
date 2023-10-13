#include "MaterialAsset.h"
#include "Render.h"
#include "Game.h"
#include "Assets.h"
#include "ShaderAsset.h"
#include "TextureAsset.h"

void MaterialAsset::Release() {
	resource.Release();
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
