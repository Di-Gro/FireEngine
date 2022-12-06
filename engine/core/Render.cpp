#include "Render.h"

#include <windows.h>
#include <WinUser.h>
#include <wrl.h>
#include <d3d.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <list>
#include <unordered_map>

#include "imgui\imgui.h"
#include "imgui\imgui_impl_dx11.h"

#include "Game.h"
#include "RenderTarget.h"
#include "DirectionLight.h"
#include "CameraComponent.h"
#include "MeshComponent.h"
#include "Refs.h"
#include "RenderPass.h"
#include "OldPass.h"
#include "Material.h"
#include "MaterialAlias.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

const std::string Render::shadowPass = "Shadow Pass";
const std::string Render::opaquePass = "Opaque Pass";
const std::string Render::lightingPass = "Lighting Pass";


void Render::Init(Game* game, Window* window) {
	m_game = game;

	m_device.Create(window->GetHWindow(), window->GetWidth(), window->GetHeight());
	m_mainRT.Init(m_game, window->GetWidth(), window->GetHeight(), true);	
	m_mainDS.Init(m_game, window->GetWidth(), window->GetHeight(), true);

	// ”становка стандартных render pass-ов
	m_shadowPass.Init(this);
	m_opaquePass.Init(this);
	m_oldPass.Init(this);

	AddRenderPass(Render::opaquePass, &m_opaquePass);
	
	m_opaquePass.SetRenderTargets({ &m_mainRT });
	m_oldPass.SetRenderTargets({ &m_mainRT });

}

void Render::Start() {
	m_screenQuad.Init(this, m_game->shaderAsset()->GetShader("../../data/engine/shaders/screen_quad.hlsl"));
	m_screenQuad.deffuseSRV = m_mainRT.shaderResource();
}

void Render::Destroy() {
	for (auto* renderPass : m_renderPipeline) {
		for (auto pair : renderPass->f_sortedMaterials) {
			delete pair.second.shapes;
		}
	}
}

void Render::m_Clear() {
	context()->ClearState();
	context()->RSSetViewports(1, m_device.viewport());
	context()->ClearDepthStencilView(m_mainDS.depthStencil(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void Render::Draw() {

	m_Clear();
	m_camera = m_game->mainCamera();

	m_shadowPass.Draw(m_shadowCasters);
	
	/// Old Pass

	//m_oldPass.Draw();

	/// New Pass

	for (auto* renderPass : m_renderPipeline)
		renderPass->Draw();

	///

	// Quad 
	m_device.BeginDraw();
	m_screenQuad.Draw();
	
	// ImGui
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	m_device.EndDraw();

}

Pass::ShadowCaster Render::AddShadowCaster(Component* component) {
	return m_shadowCasters.insert(m_shadowCasters.end(), component);
}

void Render::RemoveShadowCaster(Pass::ShadowCaster shadowCaster) {
	m_shadowCasters.erase(shadowCaster);
}

void Render::CreateTexture(
	const ImageAsset::Image* image,
	Material::Texture& tex,
	bool useSRGB,
	bool generateMips)
{
	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = image->width;
	desc.Height = image->height;
	desc.ArraySize = 1;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags = 0;
	desc.Format = useSRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.MipLevels = generateMips ? 0 : 1;
	desc.MiscFlags = generateMips ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;

	if (generateMips)
		desc.BindFlags = desc.BindFlags | D3D11_BIND_RENDER_TARGET;

	D3D11_SUBRESOURCE_DATA data = {};
	data.pSysMem = image->data;
	data.SysMemPitch = image->lineSize;
	data.SysMemSlicePitch = image->dataSize;

	auto hres = device()->CreateTexture2D(&desc, generateMips ? nullptr : &data, tex.texture2D.GetAddressOf());
	assert(SUCCEEDED(hres));

	hres = device()->CreateShaderResourceView(tex.texture2D.Get(), nullptr, tex.srv.GetAddressOf());
	assert(SUCCEEDED(hres));

	if (generateMips) {
		context()->UpdateSubresource(tex.texture2D.Get(), 0, nullptr, image->data, image->lineSize, image->dataSize);
		context()->GenerateMips(tex.srv.Get());
	}
}

int Render::m_GetRenderPassIndex(const std::string& name) {
	auto it = m_renderPassIndex.find(name);
	if (it != m_renderPassIndex.end())
		return it->second;
	return -1;
}

void Render::AddRenderPass(const std::string& name, RenderPass* renderPass, const std::string& afterRenderPass) {
	int pos = m_GetRenderPassIndex(afterRenderPass);
	assert(pos > 0);

	AddRenderPass(name, renderPass, pos + 1);
}

void Render::AddRenderPass(const std::string& name, RenderPass* renderPass, int position) {
	assert(renderPass->name() == ""); // Ётот renderPass уже добавлен
		
	int index = position >= 0 ? position : m_renderPipeline.size();

	m_renderPipeline.insert(m_renderPipeline.begin() + index, renderPass);
	m_renderPassIndex[name] = index;

	renderPass->f_name(name);
}

void Render::RemoveRenderPass(const std::string& name) {
	auto it = m_renderPassIndex.find(name);
	if (it != m_renderPassIndex.end()) {

		m_renderPipeline[it->second]->f_name("");
		m_renderPassIndex.erase(name);
		m_renderPipeline.erase(m_renderPipeline.begin() + it->second);
	}
}

RenderPass* Render::GetRenderPass(const std::string& name) {
	auto it = m_renderPassIndex.find(name);
	if (it != m_renderPassIndex.end())
		return m_renderPipeline[it->second];

	return nullptr;
}

Pass::ShapeIter Render::RegisterShape(const Material* material, MeshComponent* component, int shapeIndex) {
	assert(material != nullptr);

	// ƒобавл€ем материал, если его нет
	if (material->f_passIndex < 0) {
		auto renderPassIndex = m_GetRenderPassIndex(material->renderPass);
		assert(renderPassIndex >= 0); // Render Pass, который нужен этому материалу не добавлен.

		Pass::MatShapes matShapes;
		matShapes.material = material;
		matShapes.shapes = new Pass::ShapeList();

		auto renderPass = m_renderPipeline[renderPassIndex];
		auto materialIter = renderPass->f_sortedMaterials.insert({ material->priority, matShapes });

		material->f_materialIter = materialIter;
		material->f_passIndex = renderPassIndex;
	}

	// ƒобавл€ем shape его материалу в render pass-е
	Pass::MatShapes *matShapes = &material->f_materialIter->second;

	auto shapeRef = Pass::ShapeRef(component, shapeIndex);
	auto shapeIter = matShapes->shapes->insert(matShapes->shapes->end(), shapeRef);

	return shapeIter;
}

void Render::UnRegisterShape(const Material* material, Pass::ShapeIter& iterator) {
	if (material == nullptr)
		return;

	if (!CppRefs::IsValidPointer(material)) {
		/// THROW: 
		throw std::exception("Perhaps a dynamic material was deleted but not removed from a MeshComponent. \
			Add a call to MeshComponent.RemoveMaterial() before deleting a dynamic material.");
	}

	if (material->f_passIndex < 0)
		return;

	auto* shapes = material->f_materialIter->second.shapes;

	// ”дал€ем shape дл€ этого материала
	if (iterator != shapes->end())
		shapes->erase(iterator);
		
	// ”длл€ем материал, если у него больше нет shape-ов
	if (shapes->empty())
		UnRegisterMaterial(material);
}

void Render::UnRegisterMaterial(const Material* material) {
	if (material == nullptr)
		return;

	if (material->f_passIndex < 0)
		return;

	auto* renderPass = m_renderPipeline[material->f_passIndex];
	auto* shapes = material->f_materialIter->second.shapes;

	delete shapes;
	
	renderPass->f_sortedMaterials.erase(material->f_materialIter);

	material->f_materialIter = renderPass->f_sortedMaterials.end();
	material->f_passIndex = -1;
}

void Render::m_Draw() {
	for (auto it = m_game->BeginActor(); it != m_game->EndActor(); it++)
		(*it)->f_Draw();
}

void Render::m_DrawUI() {
	for (auto* component : m_uiDrawers) {
		if (!component->IsDestroyed())
			component->OnDraw();
	}
}