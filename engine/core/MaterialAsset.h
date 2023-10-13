#pragma once

#include <string>
#include <unordered_map>
#include <filesystem>

#include "wrl.h_d3d11_alias.h"
#include "CSBridge.h"
#include "SimpleMath.h"

#include "IAsset.h"
#include "ImageResource.h"
#include "MaterialResource.h"

namespace fs = std::filesystem;

class TextureAsset;

class MaterialAsset : public IAsset {
	friend class Render;
	friend class SceneRenderer;

public:
	bool isDynamic = false;

	std::vector<TextureAsset*> textures;
	MaterialResource resource;

private:
	std::string m_name;
	char m_name_cstr[80];

public:
	std::string name() const { return m_name; }

	void name(std::string value) {
		assert(value.size() <= 80);
		m_name = value;
	}

	void Release() override;
	
};

PUSH_ASSET(MaterialAsset);
FUNC(MaterialAsset, Init, void)(CppRef gameRef, CppRef matRef);

PROP_GETSET(MaterialAsset, Vector3, diffuseColor)
PROP_GETSET(MaterialAsset, float, diffuse)
PROP_GETSET(MaterialAsset, float, ambient)
PROP_GETSET(MaterialAsset, float, specular)
PROP_GETSET(MaterialAsset, float, shininess)
PROP_GETSET(MaterialAsset, CullMode, cullMode)
PROP_GETSET(MaterialAsset, FillMode, fillMode)
PROP_GETSET(MaterialAsset, size_t, priority)

/// TODO: «аменить работу со строками на новый вариант. 
FUNC(MaterialAsset, name_set, void)(CppRef matRef, const char* name);
FUNC(MaterialAsset, name_length, size_t)(CppRef matRef);
FUNC(MaterialAsset, name_get, void)(CppRef matRef, char* buf);

FUNC(MaterialAsset, shader_set, void)(CppRef gameRef, CppRef matRef, const char* name);
FUNC(MaterialAsset, shader_length, size_t)(CppRef matRef);
FUNC(MaterialAsset, shader_get, void)(CppRef matRef, char* buf);

FUNC(MaterialAsset, isDynamic_get, bool)(CppRef matRef);
FUNC(MaterialAsset, textures_set, void)(CppRef matRef, size_t* cppRefs, int count);