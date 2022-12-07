#pragma once
#include <initializer_list>
#include <utility>
#include <vector>
#include <string>

#include <directxmath.h>
#include <d3d11.h>

#include "wrl.h_d3d11_alias.h"
#include "MaterialAlias.h"

class Game;
class Render;
class RenderTarget;
class ShaderResource;
class DepthStencil;

class RenderPass {
	friend class Render;

#define RTCount 8
#define SRCount 6

#define Res_RenderPass_PS 0
#define Res_Material_PS SRCount

#define Buf_OpaquePass_Light_PS 0
#define Buf_Material_PS 1
#define Buf_Mesh_VS 2
#define Buf_Mesh_PS 2

protected: /// For class friends

	Pass::SortedMaterials f_sortedMaterials;
	void f_name(const std::string& value) { m_name = value; }

public:
	bool callPixelShader = true;

	D3D11_BLEND_DESC blendStateDesc;

protected:
	Render* m_render;

private:
	std::string m_name;

	RenderTarget* m_targets[RTCount] = { nullptr };
	ShaderResource* m_VSResources[SRCount] = { nullptr };
	ShaderResource* m_PSResources[SRCount] = { nullptr };

	DepthStencil* m_depthStencil = nullptr;
	
	ID3D11RenderTargetView* m_dxTargets[RTCount] = { nullptr };
	ID3D11ShaderResourceView* m_dxVSResources[SRCount] = { nullptr };
	ID3D11ShaderResourceView* m_dxPSResources[SRCount] = { nullptr };

	ID3D11SamplerState* m_dxVSSamplers[SRCount] = { nullptr };
	ID3D11SamplerState* m_dxPSSamplers[SRCount] = { nullptr };
	
	comptr<ID3D11BlendState> m_blendState;
		
public:
	virtual ~RenderPass();

	const std::string& name() { return m_name; }
	inline ID3D11DepthStencilView* depthStencil();

	void SetRenderTargets(std::initializer_list<RenderTarget*> targets);

	void SetVSShaderResources(std::initializer_list<ShaderResource*> resources);
	void SetPSShaderResources(std::initializer_list<ShaderResource*> resources);

	void SetDepthStencil(DepthStencil* depthStencil) { m_depthStencil = depthStencil; }

	virtual void Init(Game* game);
	virtual void Draw();

protected:
	inline void BeginDraw();
	inline void EndDraw();

	void PrepareMaterial(const Material* material);

	void UpdateBlendState();

private:
	inline void m_PrepareResources();
	inline void m_PrepareTargets();
	inline void m_ClearTargets();

	inline void m_PrepareMaterialResources(const Material* material);
	inline void m_SetShader(const Material* material);
	inline void m_SetMaterialConstBuffer(const Material* material);

};

