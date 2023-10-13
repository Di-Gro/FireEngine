#pragma once
#include <vector>

#include <directxmath.h>
#include <d3d11.h>
#include <SimpleMath.h>

#include "wrl.h_d3d11_alias.h"
#include "Shader.h"

using namespace DirectX::SimpleMath;

class ShaderResource;
class Render;
class Shader;

class ScreenQuad {
public:
	mutable D3D_PRIMITIVE_TOPOLOGY topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

	ShaderResource* deffuse = nullptr;
	comptr<ID3D11RasterizerState> rastState;

public:
	Render* m_render;
	const Shader* shader;
	comptr<ID3D11SamplerState> m_sampler;

public:
	void Init(Render* render, const Shader* shader);
	void Release();
	void Draw() const;
	void Draw2() const;
};