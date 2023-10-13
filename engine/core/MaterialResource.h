#pragma once

#include <string>
#include <vector>
#include <filesystem>

#include "SimpleMath.h"
#include "wrl.h_d3d11_alias.h"
#include "CsLink.h"
#include "CSBridge.h"
#include "MaterialAlias.h"
#include "ShaderResource.h"
#include "IAsset.h"

class Shader;
class Render;
class SceneRenderer;

enum class CullMode { Front, Back, None };
enum class FillMode { Solid, Wireframe };

namespace fs = std::filesystem;

class MaterialResource {

#pragma pack(push, 4)
public: struct Data {
	Vector3 diffuseColor = { 0.5, 0.5, 0.5 };		// 12
	float diffuse = 1.0f;		// Kd			// 4
	float ambient = 0.8f;		// Ka			// 4
	float specular = 0.5f;		// Ks			// 4
	float shininess = 250.0f;	// spec pow Ns	// 4
	float _1[1];
};
#pragma pack(pop)

public:
	const Shader* shader;

	std::string renderPass = "Opaque Pass";
	size_t priority = 2000;
	CullMode cullMode = CullMode::Back;
	FillMode fillMode = FillMode::Solid;
	Data data;

	std::vector<ShaderResource> textures;

	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;

	comptr<ID3D11Buffer> materialConstBuffer;
	comptr<ID3D11DepthStencilState> depthStencilState;

private:
	Render* m_render;

public:
	void Init(Render* render);
	void Release();

	void UpdateDepthStencilState();

};


