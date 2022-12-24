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

class Shader;
class Render;

enum class CullMode { Front, Back, None };
enum class FillMode { Solid, Wireframe };

class Material : public CsLink {
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

	std::vector<Texture> textures;
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

	void UpdateDepthStencilState();
};

FUNC(Material, diffuseColor_get, Vector3)(CppRef matRef);
FUNC(Material, diffuse_get, float)(CppRef matRef);
FUNC(Material, ambient_get, float)(CppRef matRef);
FUNC(Material, specular_get, float)(CppRef matRef);
FUNC(Material, shininess_get, float)(CppRef matRef);

FUNC(Material, diffuseColor_set, void)(CppRef matRef, Vector3 value);
FUNC(Material, diffuse_set, void)(CppRef matRef, float value);
FUNC(Material, ambient_set, void)(CppRef matRef, float value);
FUNC(Material, specular_set, void)(CppRef matRef, float value);
FUNC(Material, shininess_set, void)(CppRef matRef, float value);

FUNC(Material, name_get, int)(CppRef matRef, char* buf, int bufLehgth);

//extern "C" __declspec(dllexport) char* _cdecl Material_name_get(CppRef matRef);

//class DynamicMaterial
