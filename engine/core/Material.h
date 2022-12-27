#pragma once

#include <string>
#include <vector>

#include "SimpleMath.h"
#include "wrl.h_d3d11_alias.h"
#include "FileSystem.h"
#include "CsLink.h"
#include "CSBridge.h"
#include "MaterialAlias.h"
#include "ShaderResource.h"
#include "IAsset.h"

class Shader;
class Render;

enum class CullMode { Front, Back, None };
enum class FillMode { Solid, Wireframe };

class Material : public CsLink, public IAsset {
	friend class Render;

	#pragma pack(push, 4)
	public: struct Data {
		Vector3 diffuseColor = Vector3::One;		// 12
		float diffuse = 1.0f;		// Kd			// 4
		float ambient = 0.8f;		// Ka			// 4
		float specular = 0.5f;		// Ks			// 4
		float shininess = 250.0f;	// spec pow Ns	// 4
		float _1[1];
	};
	#pragma pack(pop)

public:
	std::string renderPass = "Opaque Pass";
	size_t priority = 2000;
	const Shader* shader;
	bool isDynamic = false;

	std::vector<Texture*> textures;
	std::vector<ShaderResource> resources;

	Data data;

	CullMode cullMode = CullMode::Back;
	FillMode fillMode = FillMode::Solid;

	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;

	comptr<ID3D11Buffer> materialConstBuffer;
	comptr<ID3D11DepthStencilState> depthStencilState;

private:
	mutable int f_passIndex = -1;
	mutable Pass::MaterialIter f_materialIter;

private:
	std::string m_name;
	char m_name_cstr[80];
	Render* m_render;
	
public:
	std::string name() const {  return m_name;  }

	void name(std::string value) { 
		assert(value.size() <= 80);
		m_name = value; 
	}

	void Init(Render* render);
	void Init2(Render* render);
	void Release() override;

	void UpdateDepthStencilState();
};

PUSH_ASSET(Material);
FUNC(Material, Init, void)(CppRef gameRef, CppRef matRef);

PROP_GETSET(Material, int, assetIdHash);

PROP_GETSET(Material, Vector3, diffuseColor)
PROP_GETSET(Material, float, diffuse)
PROP_GETSET(Material, float, ambient)
PROP_GETSET(Material, float, specular)
PROP_GETSET(Material, float, shininess)
PROP_GETSET(Material, CullMode, cullMode)
PROP_GETSET(Material, FillMode, fillMode)
PROP_GETSET(Material, size_t, priority)

FUNC(Material, name_set, void)(CppRef matRef, const char* name);
FUNC(Material, name_length, size_t)(CppRef matRef);
FUNC(Material, name_get, void)(CppRef matRef, char* buf);

FUNC(Material, shader_set, void)(CppRef gameRef, CppRef matRef, const char* name);
FUNC(Material, shader_length, size_t)(CppRef matRef);
FUNC(Material, shader_get, void)(CppRef matRef, char* buf);

FUNC(Material, isDynamic_get, bool)(CppRef matRef);
FUNC(Material, textures_set, void)(CppRef matRef, size_t* cppRefs, int count);

