#pragma once

#include <list>
#include <unordered_map>

#include "SimpleMath.h"
#include "MaterialAlias.h"

#include "Texture.h"
#include "ShaderResource.h"
#include "RenderTarget.h"
#include "DepthStencil.h"

class Game;
class Scene;
class CameraComponent;
class Material;

class RenderPass;
class ShadowPass;
class OpaquePass;
class SelectionPass;
class LightingPass;
class OutlinePass;
class BlurPass;

class SceneRenderer {
	friend class ShadowPass;
	friend class OldPass;
	friend class RenderPass;
	friend class LightingPass;
	friend class RenderPassUI;

private:
	class MaterialLink {
	public:
		RenderPass* renderPass;
		Pass::MaterialIter materialIter;
	};

private:
	Game* m_game = nullptr;
	Scene* m_scene = nullptr;

	Texture m_mainTexure;
	RenderTarget m_mainTarget;
	ShaderResource m_mainResource;

	Texture m_mainDepthTexure;
	DepthStencil m_mainDepthStencil;
	ShaderResource m_mainDepthResource;

	std::list<IShadowCaster*> m_shadowCasters;
	std::list<ILightSource*> m_lightSources;

	ShadowPass* m_shadowPass;
	OpaquePass* m_opaquePass;
	SelectionPass* m_highlightPass;
	LightingPass* m_lightingPass;
	OutlinePass* m_outlinePass;
	BlurPass* m_blurPass;

	std::unordered_map<std::string, int> m_renderPassIndex;
	std::vector<RenderPass*> m_renderPipeline;
	std::unordered_map<const Material*, MaterialLink>  m_linkedMaterials;

	Vector2 m_newViewportSize = { 800, 800 };
	bool m_viewportChanged = false;

	CameraComponent* m_camera = nullptr;

public:
	SceneRenderer();
	~SceneRenderer();

	void Init(Game* game, Scene* scene);
	void Start();
	void Destroy();

	void Draw();

	inline CameraComponent* camera() { return m_camera; }

	inline ID3D11DepthStencilView* depthStencil() { return m_mainDepthStencil.get(); }
	inline ID3D11ShaderResourceView* screenSRV() { return m_mainResource.get(); }

	inline Texture* depthTexture() { return &m_mainDepthTexure; }
	inline ShaderResource* depthRes() { return &m_mainDepthResource; }

	Texture* idsTexture();

	inline Vector2 viewportSize() { return m_newViewportSize; }

	inline ShadowPass* shadowPass() { return m_shadowPass; }
	inline OpaquePass* opaquePass() { return m_opaquePass; }

	void ResizeViewport(const Vector2& size);
	void ResizeViewport(float width, float height);

	void AddRenderPass(const std::string& name, RenderPass* renderPass, const std::string& afterRenderPass);
	void AddRenderPass(const std::string& name, RenderPass* renderPass, int afterIndex = -1);
	void RemoveRenderPass(const std::string& name);

	RenderPass* GetRenderPass(const std::string& name);

	Pass::ShapeIter RegisterShape(const Material* material, MeshComponent* component, int shapeIndex);
	void UnRegisterShape(const Material* material, Pass::ShapeIter& iterator);
	void UnRegisterMaterial(const Material* material);

	Pass::ShadowCaster AddShadowCaster(IShadowCaster* component);
	void RemoveShadowCaster(Pass::ShadowCaster shadowCaster);

	Pass::LightSource AddLightSource(ILightSource* component);
	void RemoveLightSource(Pass::LightSource lightSource);

	UINT GetActorIdInViewport(const Vector2& viewportPosition);

private:
	void m_Clear();
	void m_UpdatePassesInput();

	int m_GetRenderPassIndex(const std::string& name);

	void m_ResizeMainResouces(float width, float height);

public:
	void m_UpdateViewportSize();
};

