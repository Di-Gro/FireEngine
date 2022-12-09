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
#include "RenderPassUI.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

const std::string Render::shadowPassName = "Shadow Pass";
const std::string Render::opaquePassName = "Opaque Pass";
const std::string Render::lightingPassName = "Lighting Pass";


void Render::Init(Game* game, Window* window) {
	m_game = game;

	m_device.Create(window->GetHWindow(), window->GetWidth(), window->GetHeight());

	m_mainTexure = Texture::Create(this, window->GetWidth(), window->GetHeight());
	m_mainTarget = RenderTarget::Create(&m_mainTexure);
	m_mainResource = ShaderResource::Create(&m_mainTexure);

	m_mainDepthTexure = Texture::CreateDepthTexture(this, window->GetWidth(), window->GetHeight());
	m_mainDepthStencil = DepthStencil::Create(&m_mainDepthTexure);
	m_mainDepthResource = ShaderResource::Create(&m_mainDepthTexure);
	
	CD3D11_RASTERIZER_DESC rastDesc = {};
	rastDesc.CullMode = D3D11_CULL_BACK;
	rastDesc.FillMode = D3D11_FILL_SOLID;
	device()->CreateRasterizerState(&rastDesc, rastCullBack.GetAddressOf());

	rastDesc.CullMode = D3D11_CULL_FRONT;
	device()->CreateRasterizerState(&rastDesc, rastCullFront.GetAddressOf());
}

void Render::Start() {
	m_shadowPass.Init(m_game);
	m_opaquePass.Init(m_game);
	m_lightingPass.Init(m_game);

	AddRenderPass(Render::opaquePassName, &m_opaquePass);
	AddRenderPass(Render::lightingPassName, &m_lightingPass);

	m_screenQuad.Init(this, m_game->shaderAsset()->GetShader("../../data/engine/shaders/rp_screen_quad.hlsl"));
	m_screenQuad.deffuseSRV = m_mainResource.get();
	//m_screenQuad.deffuseSRV = m_opaquePass.target0Res.get();

	m_game->CreateActor("Render Pass UI")->AddComponent<RenderPassUI>();
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
	context()->ClearDepthStencilView(m_mainDepthStencil.get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}
bool once = true;

void Render::Draw() {

	if (once) {
		once = false;

		auto shadowMap = m_game->lighting()->directionLight()->depthResource();

		m_opaquePass.SetDepthStencil(&m_mainDepthStencil);
		m_opaquePass.SetRenderTargets({ 
			&m_opaquePass.target0,
			&m_opaquePass.target1, 
			&m_opaquePass.target2, 
			&m_opaquePass.target3, 
			&m_opaquePass.target4 
		});

		m_lightingPass.SetRenderTargets({ &m_mainTarget });
		m_lightingPass.SetPSShaderResources({ 
			shadowMap,
			&m_opaquePass.target0Res, 
			&m_opaquePass.target1Res, 
			&m_opaquePass.target2Res, 
			&m_opaquePass.target3Res,
			&m_opaquePass.target4Res,
		});
	}

	m_Clear();
	m_camera = m_game->mainCamera();

	m_shadowPass.Draw(m_shadowCasters);
	
	//for (auto* renderPass : m_renderPipeline)
	//	renderPass->Draw();

	m_OpaquePass();
	m_lightingPass.Draw();

	// Quad 
	m_device.BeginDraw();
	context()->RSSetState(nullptr/*rastCullBack.Get()*/);

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
	if (name == "Shadow Pass")
		return &m_shadowPass;

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

void Render::m_OpaquePass() {

	auto shadowMap = m_game->lighting()->directionLight()->depthResource();

	/// RenderPass: PrepareTargets
	ID3D11RenderTargetView* dxTargets[RTCount] = {
		m_opaquePass.target0.get(),
		m_opaquePass.target1.get(),
		m_opaquePass.target2.get(),
		m_opaquePass.target3.get(),
		m_opaquePass.target4.get(),
		nullptr,
		nullptr,
		nullptr,
	};

	/// RenderPass: PrepareResources
	ID3D11ShaderResourceView* dxResources[SRCount] = { nullptr };
	ID3D11SamplerState* dxSamplers[SRCount] = { nullptr };

	/// RenderPass: ClearTargets
	m_opaquePass.target0.Clear();
	m_opaquePass.target1.Clear();
	m_opaquePass.target2.Clear();
	m_opaquePass.target3.Clear();
	m_opaquePass.target4.Clear();

	/// RenderPass: SetCameraConstBuffer
	context()->PSSetConstantBuffers(Buf_RenderPass_Camera_PS, 1, m_opaquePass.m_cameraBuffer.GetAddressOf());

	D3D11_MAPPED_SUBRESOURCE res = {};
	context()->Map(m_opaquePass.m_cameraBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
	auto* cbuf = (CameraCBuffer*)res.pData;
	cbuf->position = camera()->worldPosition();
	context()->Unmap(m_opaquePass.m_cameraBuffer.Get(), 0);

	/// RenderPass: SetTargets
	context()->OMSetRenderTargets(8, dxTargets, m_mainDepthStencil.get());

	/// RenderPass: SetResources
	context()->VSSetShaderResources(0, SRCount, dxResources);
	context()->PSSetShaderResources(0, SRCount, dxResources);
	context()->VSSetSamplers(0, SRCount, dxSamplers);
	context()->PSSetSamplers(0, SRCount, dxSamplers);
		
	/// OpaquePass: Draw
	for (auto& pair : m_opaquePass.f_sortedMaterials) {
		auto* matShapes = &pair.second;
		auto* material = matShapes->material;
		auto* shapes = matShapes->shapes;

		bool once = true;

		for (auto shapeRef : *shapes) {
			auto* meshComponent = shapeRef.first;
			auto index = shapeRef.second;

			if (!meshComponent->IsDestroyed() && meshComponent->IsDrawable()) {
				if (once) {
					once = false;

					/// Material: SetResources
					for (int i = 0; i < material->resources.size(); ++i) {
						auto& resource = material->resources[i];

						context()->PSSetShaderResources(SRCount + i, 1, resource.getRef());
						context()->PSSetSamplers(SRCount + i, 1, resource.samplerRef());
					}
					/// Material: SetShader
					auto* shader = material->shader;
					context()->IASetInputLayout(shader->layout.Get());
					context()->VSSetShader(shader->vertex.Get(), nullptr, 0);
					context()->PSSetShader(shader->pixel.Get(), nullptr, 0);

					/// Material: SetMaterialConstBuffer
					context()->PSSetConstantBuffers(Buf_OpaquePass_Material_PS, 1, material->materialConstBuffer.GetAddressOf());

					D3D11_MAPPED_SUBRESOURCE res = {};
					context()->Map(material->materialConstBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
					memcpy(res.pData, &material->data, sizeof(Material::Data));
					context()->Unmap(material->materialConstBuffer.Get(), 0);

					/// Material: SetRasterizerState
					//context()->RSSetState(material->rastState.Get());
					context()->RSSetState(rastCullFront.Get());
				}
				meshComponent->OnDrawShape(index);
			}
		}
	}

	/// RenderPass: EndDraw
	ID3D11RenderTargetView* targets[RTCount] = { nullptr };
	context()->OMSetRenderTargets(8, targets, nullptr);

	ID3D11ShaderResourceView* resources[SRCount] = { nullptr };
	context()->VSSetShaderResources(0, SRCount, resources);
	context()->PSSetShaderResources(0, SRCount, resources);

}