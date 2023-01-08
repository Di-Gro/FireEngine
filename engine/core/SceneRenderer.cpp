#include "SceneRenderer.h"

#include "Game.h"
#include "Window.h"
#include "Render.h"
#include "RenderDevice.h"
#include "Scene.h"
#include "DirectionLight.h"

#include "RenderPass.h"
#include "ShadowPass.h"
#include "LightingPass.h"
#include "OldPass.h"
#include "OpaquePass.h"
#include "SelectionPass.h"
#include "OutlinePass.h"
#include "BlurPass.h"

//#include "Texture.h"
//#include "ShaderResource.h"
//#include "RenderTarget.h"
//#include "DepthStencil.h"

#include "Material.h"


SceneRenderer::SceneRenderer() {
	m_shadowPass = new ShadowPass();
	m_opaquePass = new OpaquePass();
	m_lightingPass = new LightingPass();
	m_highlightPass = new SelectionPass();
	m_blurPass = new BlurPass();
	m_outlinePass = new OutlinePass();
}

SceneRenderer::~SceneRenderer() {
	delete m_shadowPass;
	delete m_opaquePass;
	delete m_lightingPass;
	delete m_highlightPass;
	delete m_blurPass;
	delete m_outlinePass;
}

void SceneRenderer::Init(Game* game, Scene* scene) {
	m_game = game;
	m_scene = scene;

	auto* window = m_game->window();
	m_ResizeMainResouces(window->GetWidth(), window->GetHeight());
}

void SceneRenderer::Start() {
	m_shadowPass->Init(m_game);
	m_opaquePass->Init(m_game);
	m_lightingPass->Init(m_game);
	m_highlightPass->Init(m_game);
	m_blurPass->Init(m_game);
	m_outlinePass->Init(m_game);

	AddRenderPass("Shadow Pass", m_shadowPass);
	AddRenderPass(Render::opaquePassName, m_opaquePass);
	AddRenderPass(Render::lightingPassName, m_lightingPass);
	AddRenderPass("Editor Highlight Pass", m_highlightPass);
	AddRenderPass("Editor Blur Pass", m_blurPass);
	AddRenderPass("Editor Outline Pass", m_outlinePass);
}

void SceneRenderer::Destroy() {
	for (auto* renderPass : m_renderPipeline) {
		for (auto pair : renderPass->f_sortedMaterials) {
			delete pair.second.shapes;
		}
	}
}

void SceneRenderer::m_Clear() {
	auto* render = m_game->render();

	render->context()->ClearState();
	render->rdevice()->SetViewport();
	render->context()->ClearDepthStencilView(m_mainDepthStencil.get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void SceneRenderer::m_UpdatePassesInput() {

	m_opaquePass->SetDepthStencil(&m_mainDepthStencil);
	m_opaquePass->SetRenderTargets({
		&m_opaquePass->target0,
		&m_opaquePass->target1,
		&m_opaquePass->target2,
		&m_opaquePass->target3,
		&m_opaquePass->target4,
		&m_opaquePass->target5,
		});

	auto dirLight = m_scene->directionLight;
	auto shadowMap = dirLight != nullptr ? dirLight->depthResource() : nullptr;

	m_lightingPass->SetRenderTargets({ &m_mainTarget });
	m_lightingPass->SetPSShaderResources({
		shadowMap,
		&m_opaquePass->target0Res,
		&m_opaquePass->target1Res,
		&m_opaquePass->target2Res,
		&m_opaquePass->target3Res,
		&m_opaquePass->target4Res,
		});

	m_blurPass->SetPSShaderResources({ &m_highlightPass->target0Res });

	m_outlinePass->SetRenderTargets({ &m_mainTarget });
	m_outlinePass->SetPSShaderResources({ &m_highlightPass->target0Res, &m_blurPass->target0Res });
	m_outlinePass->clearTargets = false;
}

void SceneRenderer::Draw() {
	m_Clear();

	m_camera = m_scene->mainCamera();
	if (m_camera == nullptr)
		return;

	m_game->PushScene(m_scene);

	m_UpdatePassesInput();

	for (auto* renderPass : m_renderPipeline)
		renderPass->Draw();

	m_game->PopScene();

	auto vsize = viewportSize();
	m_game->render()->rdevice()->ResizeViewport(vsize.x, vsize.y);
}

void SceneRenderer::m_ResizeMainResouces(float width, float height) { 
	m_mainTexure = Texture::Create(m_game->render(), width, height);
	m_mainTarget = RenderTarget::Create(&m_mainTexure);
	m_mainResource = ShaderResource::Create(&m_mainTexure);

	m_mainDepthTexure = Texture::CreateDepthTexture(m_game->render(), width, height);
	m_mainDepthStencil = DepthStencil::Create(&m_mainDepthTexure);
	m_mainDepthResource = ShaderResource::Create(&m_mainDepthTexure);
}

Texture* SceneRenderer::idsTexture() {
	if (m_opaquePass != nullptr)
		return &m_opaquePass->target5Tex;
	return nullptr;
}

Texture* SceneRenderer::wposTexture() {
	if (m_opaquePass != nullptr)
		return &m_opaquePass->target3Tex;
	return nullptr;
}

void SceneRenderer::ResizeViewport(const Vector2& size) {
	ResizeViewport(size.x, size.y);
}

void SceneRenderer::ResizeViewport(float width, float height) {
	if (m_newViewportSize != Vector2(width, height)) {
		if (width < 2) width = 2;
		if (height < 2) height = 2;

		m_viewportChanged = true;
		m_newViewportSize = { width, height };
	}
}

void SceneRenderer::m_UpdateViewportSize() {
	if (m_viewportChanged) {
		m_viewportChanged = false;

		float width = m_newViewportSize.x;
		float height = m_newViewportSize.y;

		m_ResizeMainResouces(width, height);

		for (auto* renderPass : m_renderPipeline)
			renderPass->Resize(width, height);
	}
}

void SceneRenderer::AddRenderPass(const std::string& name, RenderPass* renderPass, const std::string& afterRenderPass) {
	int pos = m_GetRenderPassIndex(afterRenderPass);
	assert(pos > 0);

	AddRenderPass(name, renderPass, pos + 1);
}

void SceneRenderer::AddRenderPass(const std::string& name, RenderPass* renderPass, int position) {
	assert(renderPass->name() == ""); // Ётот renderPass уже добавлен

	int index = position >= 0 ? position : m_renderPipeline.size();

	m_renderPipeline.insert(m_renderPipeline.begin() + index, renderPass);
	m_renderPassIndex[name] = index;

	renderPass->f_name(name);
}

void SceneRenderer::RemoveRenderPass(const std::string& name) {
	auto it = m_renderPassIndex.find(name);
	if (it != m_renderPassIndex.end()) {

		m_renderPipeline[it->second]->f_name("");
		m_renderPassIndex.erase(name);
		m_renderPipeline.erase(m_renderPipeline.begin() + it->second);
	}
}

RenderPass* SceneRenderer::GetRenderPass(const std::string& name) {
	if (name == "Shadow Pass")
		return m_shadowPass;

	auto it = m_renderPassIndex.find(name);
	if (it != m_renderPassIndex.end())
		return m_renderPipeline[it->second];

	return nullptr;
}

int SceneRenderer::m_GetRenderPassIndex(const std::string& name) {
	auto it = m_renderPassIndex.find(name);
	if (it != m_renderPassIndex.end())
		return it->second;
	return -1;
}

Pass::ShapeIter SceneRenderer::RegisterShape(const Material* material, MeshComponent* component, int shapeIndex) {
	assert(material != nullptr);

	auto cppRef = CppRefs::GetRef((void*)material);

	// ƒобавл€ем материал, если его нет
	if (!m_linkedMaterials.contains(material)) {
		auto renderPassIndex = m_GetRenderPassIndex(material->renderPass);
		assert(renderPassIndex >= 0); // Render Pass, который нужен этому материалу не добавлен.

		Pass::MatShapes matShapes;
		matShapes.material = material;
		matShapes.shapes = new Pass::ShapeList();

		auto renderPass = m_renderPipeline[renderPassIndex];
		auto materialIter = renderPass->f_sortedMaterials.insert({ material->priority, matShapes });

		MaterialLink link;
		link.renderPass = renderPass;
		link.materialIter = materialIter;

		m_linkedMaterials.insert(std::make_pair(material, link));
	}

	auto link = m_linkedMaterials.at(material);

	// ƒобавл€ем shape его материалу в render pass-е
	Pass::MatShapes* matShapes = &link.materialIter->second;

	auto shapeRef = Pass::ShapeRef(component, shapeIndex);
	auto shapeIter = matShapes->shapes->insert(matShapes->shapes->end(), shapeRef);

	return shapeIter;
}

void SceneRenderer::UnRegisterShape(const Material* material, Pass::ShapeIter& iterator) {
	if (material == nullptr)
		return;

	auto cppRef = CppRefs::GetRef((void*)material);

	if (!CppRefs::IsValidPointer(material)) {
		/// THROW: 
		throw std::exception("Perhaps a dynamic material was deleted but not removed from a MeshComponent. \
			Add a call to MeshComponent.RemoveMaterial() before deleting a dynamic material.");
	}

	if (!m_linkedMaterials.contains(material))
		return;

	auto link = m_linkedMaterials.at(material);

	auto* shapes = link.materialIter->second.shapes;

	// ”дал€ем shape дл€ этого материала
	if (iterator != shapes->end())
		shapes->erase(iterator);

	// ”длл€ем материал, если у него больше нет shape-ов
	if (shapes->empty())
		UnRegisterMaterial(material);
}

void SceneRenderer::UnRegisterMaterial(const Material* material) {
	if (material == nullptr)
		return;

	auto cppRef = CppRefs::GetRef((void*)material);

	if (!m_linkedMaterials.contains(material))
		return;

	auto link = m_linkedMaterials.at(material);

	auto* shapes = link.materialIter->second.shapes;

	delete shapes;

	link.renderPass->f_sortedMaterials.erase(link.materialIter);

	m_linkedMaterials.erase(material);
}

Pass::ShadowCaster SceneRenderer::AddShadowCaster(IShadowCaster* component) {
	return m_shadowCasters.insert(m_shadowCasters.end(), component);
}

void SceneRenderer::RemoveShadowCaster(Pass::ShadowCaster shadowCaster) {
	m_shadowCasters.erase(shadowCaster);
}

Pass::LightSource SceneRenderer::AddLightSource(ILightSource* component) {
	return m_lightSources.insert(m_lightSources.end(), component);
}

void SceneRenderer::RemoveLightSource(Pass::LightSource lightSource) {
	m_lightSources.erase(lightSource);
}

UINT SceneRenderer::GetActorIdInViewport(const Vector2& viewportPosition) {
	return m_game->render()->GetActorIdInViewport(m_scene, viewportPosition);
}