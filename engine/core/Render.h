#pragma once

#include <list>
#include <vector>
#include <map>
#include <set>
#include <unordered_map>

#include "RenderDevice.h"
#include "RenderPass.h"
#include "ShadowPass.h"
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

class Render {
	friend class ShadowPass;
	friend class OldPass;
	friend class RenderPass;
public:
	static const std::string shadowPass;
	static const std::string opaquePass;
	static const std::string lightingPass;
	
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
	std::list<Component*> m_uiDrawers;

	std::unordered_map<std::string, int> m_renderPassIndex;
	std::vector<RenderPass*> m_renderPipeline;

	ShadowPass m_shadowPass;
	OldPass m_oldPass;
	OpaquePass m_opaquePass;

	CameraComponent* m_camera = nullptr;

public:
	inline CameraComponent* camera() { return m_camera; }

	inline ID3D11Device* device() { return m_device.GetDevice(); };
	inline ID3D11DeviceContext* context() { return m_device.GetContext(); }
	inline IDXGISwapChain* swapChain() { return m_device.GetSwapChain(); }
	inline comptr<ID3D11RenderTargetView> rtv() { return m_device.GetRTV(); }
	inline ID3D11DepthStencilView* depthStencil() { return m_mainDepthStencil.get(); }
	inline ID3D11ShaderResourceView* screenSRV() { return m_mainResource.get(); }

	void Init(Game* game, Window* window);
	void Start();
	void Destroy();
	void Draw();

	void AddRenderPass(const std::string& name, RenderPass* renderPass, const std::string& afterRenderPass);
	void AddRenderPass(const std::string& name, RenderPass* renderPass, int afterIndex = -0);
	void RemoveRenderPass(const std::string& name);

	RenderPass* GetRenderPass(const std::string& name);

	Pass::ShapeIter RegisterShape(const Material* material, MeshComponent* component, int shapeIndex);
	void UnRegisterShape(const Material* material, Pass::ShapeIter& iterator);
	void UnRegisterMaterial(const Material* material);

	Pass::ShadowCaster AddShadowCaster(Component* component);
	void RemoveShadowCaster(Pass::ShadowCaster shadowCaster);

private:
	void m_Clear();

	void m_Draw();
	void m_DrawUI();

	size_t m_GetStringHash(const std::string& str) { return std::hash<std::string>()(str); }
	int m_GetRenderPassIndex(const std::string& name);

};

