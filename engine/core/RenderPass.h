#pragma once
#include <initializer_list>
#include <utility>
#include <vector>
#include <string>

#include <directxmath.h>
#include <d3d11.h>

#include "wrl.h_d3d11_alias.h"
#include "MaterialAlias.h"

#define RTCount 8
#define SRCount 8

#define PASS_R_PASS_PS 0
#define PASS_R_MATERIAL_PS SRCount

/// CBuffers Pixel Shader
#define PASS_CB_CAMERA_PS 0
#define PASS_CB_SHADOW_PS 1
#define PASS_CB_LIGHT_PS 2
#define PASS_CB_MATERIAL_PS 3
#define PASS_CB_MESH_PS 4
#define PASS_CB_ACTOR_PS 5
#define PASS_CB_EDITOR_PS 6

/// CBuffers Vertex Shader
#define PASS_CB_MESH_VS 4

/// RenderPass: PrepareTargets
/// RenderPass: PrepareResources
/// RenderPass: ClearTargets
/// RenderPass: SetCameraConstBuffer
/// RenderPass: SetTargets
/// RenderPass: SetResources
/// RenderPass: SetBlendState
/// Material: SetResources
/// Material: SetShader
/// Material: SetMaterialConstBuffer
/// Material: SetRasterizerState
/// Mesh: SetTopology
/// Mesh: SetInput
/// Mesh: SetConstBuffers
/// Mesh: Draw
/// RenderPass: EndDraw

class Game;
class Actor;
class Render;
class RenderTarget;
class ShaderResource;
class DepthStencil;
class SceneRenderer;

class RenderPass {
	friend class Render;
	friend class SceneRenderer;

protected: /// For class friends
	Pass::SortedMaterials f_sortedMaterials;
	void f_name(const std::string& value) { m_name = value; }

public:
	bool callPixelShader = true;
	bool clearTargets = true;

	D3D11_BLEND_DESC blendStateDesc;

protected:
	Game* m_game;
	Render* m_render;

protected:
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
	comptr<ID3D11Buffer> m_cameraBuffer;
	comptr<ID3D11Buffer> m_actorBuffer;
	comptr<ID3D11Buffer> m_editorBuffer;
		
public:
	virtual ~RenderPass();

	const std::string& name() { return m_name; }
	inline DepthStencil* depthStencil() { return m_depthStencil; }

	void SetRenderTargets(std::initializer_list<RenderTarget*> targets);

	void SetVSShaderResources(std::initializer_list<ShaderResource*> resources);
	void SetPSShaderResources(std::initializer_list<ShaderResource*> resources);

	void SetDepthStencil(DepthStencil* depthStencil) { m_depthStencil = depthStencil; }

	virtual void Init(Game* game);
	virtual void Draw();
	virtual void Resize(float width, float height) { };

	void PrepareMaterial(const Material* material);
	void SetActorConstBuffer(Actor* actor);
	void SetEditorConstBuffer();

protected:
	inline void BeginDraw();
	inline void EndDraw();

	void UpdateBlendState();
	
private:
	inline void m_PrepareResources();
	inline void m_PrepareTargets();
	inline void m_ClearTargets();

	inline void m_SetCameraConstBuffer();
	
	inline void m_PrepareMaterialResources(const Material* material);
	inline void m_SetShader(const Material* material);
	inline void m_SetMaterialConstBuffer(const Material* material);

};

