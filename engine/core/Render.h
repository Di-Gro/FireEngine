#pragma once

#include <list>
#include <vector>
#include <map>
#include <set>
#include <unordered_map>

#include "SimpleMath.h"

#include "RenderDevice.h"
#include "MaterialAlias.h"

//#include "RenderPass.h"
//#include "ShadowPass.h"
//#include "LightingPass.h"
//#include "OldPass.h"
//#include "OpaquePass.h"
//#include "SelectionPass.h"
//#include "OutlinePass.h"
//#include "BlurPass.h"
//
#include "Texture.h"
//#include "ShaderResource.h"
//#include "RenderTarget.h"
//#include "DepthStencil.h"

class Game;
class Scene;
class Actor;
class Window;
class CameraComponent;
class MeshComponent;
class Component;
class Material;
class ILightSource;
class IShadowCaster;
class Material;

class ShaderResource;
class RenderTarget;
class DepthStencil;
class SceneRenderer;

class Render {
	friend class ShadowPass;
	friend class OldPass;
	friend class RenderPass;
	friend class LightingPass;
	friend class RenderPassUI;

public:
	static const std::string shadowPassName;
	static const std::string opaquePassName;
	static const std::string lightingPassName;
	static const std::string screenSpacePassName;
	
private:
	Game* m_game = nullptr;
	RenderDevice m_device;

	//Texture m_mainTexure;
	//RenderTarget m_mainTarget;
	//ShaderResource m_mainResource;

	//Texture m_mainDepthTexure;
	//DepthStencil m_mainDepthStencil;
	//ShaderResource m_mainDepthResource;

	//ScreenQuad m_screenQuad;

	Texture onePixelStagingTexUINT;
	Texture onePixelStagingTexFLOAT3;

	//std::list<IShadowCaster*> m_shadowCasters;
	//std::list<ILightSource*> m_lightSources;
	//std::list<Component*> m_uiDrawers;

	//std::unordered_map<std::string, int> m_renderPassIndex;
	//std::vector<RenderPass*> m_renderPipeline;

	//ShadowPass m_shadowPass;
	//OpaquePass m_opaquePass;
	//SelectionPass m_highlightPass;
	//LightingPass m_lightingPass;
	//OutlinePass m_outlinePass;
	//BlurPass m_blurPass;

	//CameraComponent* m_camera = nullptr;
	SceneRenderer* m_renderer = nullptr;
	Scene* m_scene = nullptr;

	//Vector2 m_newViewportSize;
	//bool m_viewportChanged = false;

private:
	comptr<ID3D11RasterizerState> m_cullSolidBack;
	comptr<ID3D11RasterizerState> m_cullSolidFront;
	comptr<ID3D11RasterizerState> m_cullSolidNone;

	comptr<ID3D11RasterizerState> m_cullWireframeBack;
	comptr<ID3D11RasterizerState> m_cullWireframeFront;
	comptr<ID3D11RasterizerState> m_cullWireframeNone;

public:
	//inline CameraComponent* camera() { return m_camera; }

	inline SceneRenderer* renderer() { return m_renderer; }
	inline Scene* scene() { return m_scene; }

	//inline Vector2 viewportSize() { return m_newViewportSize; }

	inline ID3D11Device* device() { return m_device.GetDevice(); };
	inline RenderDevice* rdevice() { return &m_device; };
	inline ID3D11DeviceContext* context() { return m_device.GetContext(); }
	inline IDXGISwapChain* swapChain() { return m_device.GetSwapChain(); }
	inline comptr<ID3D11RenderTargetView> rtv() { return m_device.GetRTV(); }
	//inline ID3D11DepthStencilView* depthStencil() { return m_mainDepthStencil.get(); }
	//inline ID3D11ShaderResourceView* screenSRV() { return m_mainResource.get(); }

	//inline Texture* depthTexture() { return &m_mainDepthTexure; }
	//inline ShaderResource* depthRes() { return &m_mainDepthResource; }

	//inline ShadowPass* shadowPass() { return &m_shadowPass; }
	//inline OpaquePass* opaquePass() { return &m_opaquePass; }

	void Init(Game* game, Window* window);
	void Start();
	void Destroy();
	void Draw(std::list<Scene*>* scenes);

	//void AddRenderPass(const std::string& name, RenderPass* renderPass, const std::string& afterRenderPass);
	//void AddRenderPass(const std::string& name, RenderPass* renderPass, int afterIndex = -1);
	//void RemoveRenderPass(const std::string& name);

	//RenderPass* GetRenderPass(const std::string& name);

	//Pass::ShapeIter RegisterShape(const Material* material, MeshComponent* component, int shapeIndex);
	//void UnRegisterShape(const Material* material, Pass::ShapeIter& iterator);
	//void UnRegisterMaterial(const Material* material);

	//Pass::ShadowCaster AddShadowCaster(IShadowCaster* component);
	//void RemoveShadowCaster(Pass::ShadowCaster shadowCaster);

	//Pass::LightSource AddLightSource(ILightSource* component);
	//void RemoveLightSource(Pass::LightSource lightSource);

	ID3D11RasterizerState* GetRastState(const Material* material);
	ID3D11RasterizerState* GetRastState(CullMode cullMode, FillMode fillMode = FillMode::Solid);

	//void ResizeViewport(const Vector2& size);
	//void ResizeViewport(float width, float height);

	UINT GetActorIdInViewport(Scene* scene, const Vector2& viewportPosition);
	Vector3 GetWposInViewport(Scene* scene, const Vector2& viewportPosition);

private:
	//void m_Clear();
	void m_UpdateDeviceSize();

	//void m_OpaquePass();
	//void m_SecondPass();

	size_t m_GetStringHash(const std::string& str) { return std::hash<std::string>()(str); }
	//int m_GetRenderPassIndex(const std::string& name);

	//void m_ResizeMainResouces(float width, float height);

	
};

