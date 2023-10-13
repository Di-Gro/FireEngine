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
#include "TextureResource.h"
#include "MaterialResource.h"
//#include "ShaderResource.h"
//#include "RenderTarget.h"
//#include "DepthStencil.h"

class Window;
class Game;
class Scene;
class Actor;
class SceneRenderer;

class Component;
class MeshComponent;
class CameraComponent;

class ILightSource;
class IShadowCaster;

class ShaderResource;
class MaterialResource;
class RenderTarget;
class DepthStencil;

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

	TextureResource onePixelStagingTexUINT;
	TextureResource onePixelStagingTexFLOAT3;

	SceneRenderer* m_renderer = nullptr;
	Scene* m_scene = nullptr;

private:
	comptr<ID3D11RasterizerState> m_cullSolidBack;
	comptr<ID3D11RasterizerState> m_cullSolidFront;
	comptr<ID3D11RasterizerState> m_cullSolidNone;

	comptr<ID3D11RasterizerState> m_cullWireframeBack;
	comptr<ID3D11RasterizerState> m_cullWireframeFront;
	comptr<ID3D11RasterizerState> m_cullWireframeNone;

public:
	inline SceneRenderer* renderer() { return m_renderer; }
	inline Scene* scene() { return m_scene; }

	inline ID3D11Device* device() { return m_device.GetDevice(); };
	inline RenderDevice* rdevice() { return &m_device; };
	inline ID3D11DeviceContext* context() { return m_device.GetContext(); }
	inline IDXGISwapChain* swapChain() { return m_device.GetSwapChain(); }
	inline comptr<ID3D11RenderTargetView> rtv() { return m_device.GetRTV(); }

	void Init(Game* game, Window* window);
	void Start();
	void Destroy();
	void Draw(std::list<Scene*>* scenes);

	ID3D11RasterizerState* GetRastState(const MaterialResource* material);
	ID3D11RasterizerState* GetRastState(CullMode cullMode, FillMode fillMode = FillMode::Solid);

	UINT GetActorIdInViewport(Scene* scene, const Vector2& viewportPosition);
	Vector3 GetWposInViewport(Scene* scene, const Vector2& viewportPosition);

private:
	void m_UpdateDeviceSize();

	size_t m_GetStringHash(const std::string& str) { return std::hash<std::string>()(str); }
	
};

