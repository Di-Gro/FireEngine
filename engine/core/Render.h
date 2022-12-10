#pragma once

#include <list>
#include <vector>
#include <map>
#include <set>
#include <unordered_map>

#include "RenderDevice.h"
#include "RenderPass.h"
#include "ShadowPass.h"
#include "LightingPass.h"
#include "OldPass.h"
#include "OpaquePass.h"
#include "DepthStencil.h"
#include "MaterialAlias.h"

#include "Texture.h"
#include "ShaderResource.h"
#include "RenderTarget.h"

class Game;
class Actor;
class Window;
class CameraComponent;
class MeshComponent;
class Component;
class Material;
class ILightSource;

class Render {
	friend class ShadowPass;
	friend class OldPass;
	friend class RenderPass;
	friend class LightingPass;
public:
	static const std::string shadowPassName;
	static const std::string opaquePassName;
	static const std::string lightingPassName;
	
private:
	Game* m_game;
	RenderDevice m_device;

	Texture m_mainTexure;
	RenderTarget m_mainTarget;
	ShaderResource m_mainResource;

	Texture m_mainDepthTexure;
	DepthStencil m_mainDepthStencil;
	ShaderResource m_mainDepthResource;

	ScreenQuad m_screenQuad;

	std::list<Component*> m_shadowCasters;
	std::list<ILightSource*> m_lightSources;
	std::list<Component*> m_uiDrawers;

	std::unordered_map<std::string, int> m_renderPassIndex;
	std::vector<RenderPass*> m_renderPipeline;

	ShadowPass m_shadowPass;
	OpaquePass m_opaquePass;
	LightingPass m_lightingPass;

	CameraComponent* m_camera = nullptr;

private:
	comptr<ID3D11RasterizerState> m_ñullSolidBack;
	comptr<ID3D11RasterizerState> m_ñullSolidFront;

	comptr<ID3D11RasterizerState> m_ñullWireframeBack;
	comptr<ID3D11RasterizerState> m_ñullWireframeFront;

public:
	inline CameraComponent* camera() { return m_camera; }

	inline RenderDevice* rdevice() { return &m_device; };
	inline ID3D11Device* device() { return m_device.GetDevice(); };
	inline ID3D11DeviceContext* context() { return m_device.GetContext(); }
	inline IDXGISwapChain* swapChain() { return m_device.GetSwapChain(); }
	inline comptr<ID3D11RenderTargetView> rtv() { return m_device.GetRTV(); }
	inline ID3D11DepthStencilView* depthStencil() { return m_mainDepthStencil.get(); }
	inline ID3D11ShaderResourceView* screenSRV() { return m_mainResource.get(); }

	inline Texture* depthTexture() { return &m_mainDepthTexure; }
	inline ShaderResource* depthRes() { return &m_mainDepthResource; }

	inline ShadowPass* shadowPass() { return &m_shadowPass; }
	inline OpaquePass* opaquePass() { return &m_opaquePass; }


	void Init(Game* game, Window* window);
	void Start();
	void Destroy();
	void Draw();

	void AddRenderPass(const std::string& name, RenderPass* renderPass, const std::string& afterRenderPass);
	void AddRenderPass(const std::string& name, RenderPass* renderPass, int afterIndex = -1);
	void RemoveRenderPass(const std::string& name);

	RenderPass* GetRenderPass(const std::string& name);

	Pass::ShapeIter RegisterShape(const Material* material, MeshComponent* component, int shapeIndex);
	void UnRegisterShape(const Material* material, Pass::ShapeIter& iterator);
	void UnRegisterMaterial(const Material* material);

	Pass::ShadowCaster AddShadowCaster(Component* component);
	void RemoveShadowCaster(Pass::ShadowCaster shadowCaster);

	Pass::LightSource AddLightSource(ILightSource* component);
	void RemoveLightSource(Pass::LightSource lightSource);

	ID3D11RasterizerState* GetRastState(const Material* material);
	ID3D11RasterizerState* GetRastState(CullMode cullMode, FillMode fillMode = FillMode::Solid);

private:
	void m_Clear();

	void m_Draw();
	void m_DrawUI();

	void m_OpaquePass();
	void m_SecondPass();

	size_t m_GetStringHash(const std::string& str) { return std::hash<std::string>()(str); }
	int m_GetRenderPassIndex(const std::string& name);

};

