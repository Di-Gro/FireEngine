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
#include "Mesh.h"

#include "ILightSource.h"
#include "IShadowCaster.h"

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

	m_ResizeMainResouces(window->GetWidth(), window->GetHeight());
	
	CD3D11_RASTERIZER_DESC rastDesc = {};
	rastDesc.CullMode = D3D11_CULL_BACK;
	rastDesc.FillMode = D3D11_FILL_SOLID;
	device()->CreateRasterizerState(&rastDesc, m_ñullSolidBack.GetAddressOf());

	rastDesc.CullMode = D3D11_CULL_FRONT;
	device()->CreateRasterizerState(&rastDesc, m_ñullSolidFront.GetAddressOf());

	rastDesc.CullMode = D3D11_CULL_BACK;
	rastDesc.FillMode = D3D11_FILL_WIREFRAME;
	device()->CreateRasterizerState(&rastDesc, m_ñullWireframeBack.GetAddressOf());

	rastDesc.CullMode = D3D11_CULL_FRONT;
	device()->CreateRasterizerState(&rastDesc, m_ñullWireframeFront.GetAddressOf());
}

void Render::m_ResizeMainResouces(float width, float height) {
	m_mainTexure = Texture::Create(this, width, height);
	m_mainTarget = RenderTarget::Create(&m_mainTexure);
	m_mainResource = ShaderResource::Create(&m_mainTexure);

	m_mainDepthTexure = Texture::CreateDepthTexture(this, width, height);
	m_mainDepthStencil = DepthStencil::Create(&m_mainDepthTexure);
	m_mainDepthResource = ShaderResource::Create(&m_mainDepthTexure);
}

void Render::Start() {
	m_shadowPass.Init(m_game);
	m_opaquePass.Init(m_game);
	m_lightingPass.Init(m_game);

	AddRenderPass("Shadow Pass", &m_shadowPass);
	AddRenderPass(Render::opaquePassName, &m_opaquePass);
	AddRenderPass(Render::lightingPassName, &m_lightingPass);

	m_screenQuad.Init(this, m_game->shaderAsset()->GetShader("../../data/engine/shaders/rp_screen_quad.hlsl"));
	m_screenQuad.deffuse = &m_mainResource;

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
	rdevice()->SetViewport();
	context()->ClearDepthStencilView(m_mainDepthStencil.get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}
bool once = true;

void Render::Draw() {

	if (once) {
		once = false;

		m_opaquePass.SetDepthStencil(&m_mainDepthStencil);
		m_opaquePass.SetRenderTargets({ 
			&m_opaquePass.target0,
			&m_opaquePass.target1, 
			&m_opaquePass.target2, 
			&m_opaquePass.target3, 
			&m_opaquePass.target4,
		});

		auto shadowMap = m_game->lighting()->directionLight()->depthResource();

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

	/// Begin Render
	m_Clear();
	m_camera = m_game->mainCamera();
		
	/// Render Passes
	for (auto* renderPass : m_renderPipeline)
		renderPass->Draw();

	/// Begin Draw
	m_device.BeginDraw();

	///// Draw Quad 
	//context()->RSSetState(GetRastState(CullMode::Back));
	//m_screenQuad.Draw();
	
	/// Draw ImGui
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	/// End Draw
	m_device.EndDraw();
	
	/// Post Draw
	m_UpdateSize();
}

Pass::ShadowCaster Render::AddShadowCaster(IShadowCaster* component) {
	return m_shadowCasters.insert(m_shadowCasters.end(), component);
}

void Render::RemoveShadowCaster(Pass::ShadowCaster shadowCaster) {
	m_shadowCasters.erase(shadowCaster);
}

Pass::LightSource Render::AddLightSource(ILightSource* component) {
	return m_lightSources.insert(m_lightSources.end(), component);
}

void Render::RemoveLightSource(Pass::LightSource lightSource) {
	m_lightSources.erase(lightSource);
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
	assert(renderPass->name() == ""); // Ýòîò renderPass óæå äîáàâëåí
		
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

	// Äîáàâëÿåì ìàòåðèàë, åñëè åãî íåò
	if (material->f_passIndex < 0) {
		auto renderPassIndex = m_GetRenderPassIndex(material->renderPass);
		assert(renderPassIndex >= 0); // Render Pass, êîòîðûé íóæåí ýòîìó ìàòåðèàëó íå äîáàâëåí.

		Pass::MatShapes matShapes;
		matShapes.material = material;
		matShapes.shapes = new Pass::ShapeList();

		auto renderPass = m_renderPipeline[renderPassIndex];
		auto materialIter = renderPass->f_sortedMaterials.insert({ material->priority, matShapes });

		material->f_materialIter = materialIter;
		material->f_passIndex = renderPassIndex;
	}

	// Äîáàâëÿåì shape åãî ìàòåðèàëó â render pass-å
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

	// Óäàëÿåì shape äëÿ ýòîãî ìàòåðèàëà
	if (iterator != shapes->end())
		shapes->erase(iterator);
		
	// Óäëëÿåì ìàòåðèàë, åñëè ó íåãî áîëüøå íåò shape-îâ
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

void Render::m_UpdateSize() {
	auto window = m_game->window();
	if (window->sizeChanged) {
		m_game->window()->sizeChanged = false;

		float width = window->GetWidth();
		float height = window->GetHeight();

		m_device.Resize(width, height);
		//ResizeViewport(width, height);
	}
	if (m_viewportChanged) {
		m_viewportChanged = false;

		float width = m_newViewportSize.x;
		float height = m_newViewportSize.y;

		m_device.ResizeViewport(width, height);

		m_ResizeMainResouces(width, height);

		for (auto* renderPass : m_renderPipeline)
			renderPass->Resize(width, height);
	}
}

void Render::ResizeViewport(float width, float height) {
	if (m_newViewportSize != Vector2(width, height)) {
		if (width < 2) width = 2;
		if (height < 2) height = 2;

		m_viewportChanged = true;
		m_newViewportSize = { width, height };
	}
}

void Render::m_OpaquePass() {
	/*
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
	auto shadowMap = m_game->lighting()->directionLight()->depthResource();

	ID3D11ShaderResourceView* dxResources[SRCount] = {
			shadowMap->get(),
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
	};
	ID3D11SamplerState* dxSamplers[SRCount] = {
			shadowMap->sampler(),
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
	};

	/// RenderPass: ClearTargets
	m_opaquePass.target0.Clear();
	m_opaquePass.target1.Clear();
	m_opaquePass.target2.Clear();
	m_opaquePass.target3.Clear();
	m_opaquePass.target4.Clear();

	/// RenderPass: SetCameraConstBuffer
	auto& cameraBuffer = m_opaquePass.m_cameraBuffer;
	context()->PSSetConstantBuffers(PASS_CB_CAMERA_PS, 1, cameraBuffer.GetAddressOf());

	D3D11_MAPPED_SUBRESOURCE res = {};
	context()->Map(cameraBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
	auto* cbuf = (CameraCBuffer*)res.pData;
	cbuf->position = camera()->worldPosition();
	context()->Unmap(cameraBuffer.Get(), 0);

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

						context()->PSSetShaderResources(PASS_R_MATERIAL_PS + i, 1, resource.getRef());
						context()->PSSetSamplers(PASS_R_MATERIAL_PS + i, 1, resource.samplerRef());
					}
					/// Material: SetShader
					auto* shader = material->shader;
					context()->IASetInputLayout(shader->layout.Get());
					context()->VSSetShader(shader->vertex.Get(), nullptr, 0);
					context()->PSSetShader(shader->pixel.Get(), nullptr, 0);

					/// Material: SetMaterialConstBuffer
					auto& materialBuffer = material->materialConstBuffer;
					context()->PSSetConstantBuffers(PASS_CB_MATERIAL_PS, 1, materialBuffer.GetAddressOf());

					D3D11_MAPPED_SUBRESOURCE res = {};
					context()->Map(materialBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
					memcpy(res.pData, &material->data, sizeof(Material::Data));
					context()->Unmap(materialBuffer.Get(), 0);

					/// Material: SetRasterizerState
					context()->RSSetState(GetRastState(material));
				}
				/// OpaquePass: DrawShape
				meshComponent->OnDrawShape(index);
			}
		}
	}

	/// RenderPass: EndDraw
	ID3D11RenderTargetView* targets[RTCount] = { nullptr };
	context()->OMSetRenderTargets(8, targets, nullptr);

	ID3D11ShaderResourceView* resources[SRCount] = { nullptr };
	context()->VSSetShaderResources(0, SRCount, resources);
	context()->PSSetShaderResources(0, SRCount, resources);*/

}

void Render::m_SecondPass() {
	
	///// RenderPass: PrepareTargets
	//ID3D11RenderTargetView* dxTargets[RTCount] = {
	//	m_mainTarget.get(),
	//	nullptr,
	//	nullptr,
	//	nullptr,
	//	nullptr,
	//	nullptr,
	//	nullptr,
	//	nullptr,
	//};

	///// RenderPass: PrepareResources
	//auto shadowMap = m_game->lighting()->directionLight()->depthResource();

	//ID3D11ShaderResourceView* dxResources[SRCount] = {
	//		shadowMap->get(),
	//		m_opaquePass.target0Res.get(),
	//		m_opaquePass.target1Res.get(),
	//		m_opaquePass.target2Res.get(),
	//		m_opaquePass.target3Res.get(),
	//		m_opaquePass.target4Res.get(),
	//		nullptr,
	//		nullptr,
	//};

	//ID3D11SamplerState* dxSamplers[SRCount] = {
	//		shadowMap->sampler(),
	//		m_opaquePass.target0Res.sampler(),
	//		m_opaquePass.target1Res.sampler(),
	//		m_opaquePass.target2Res.sampler(),
	//		m_opaquePass.target3Res.sampler(),
	//		m_opaquePass.target4Res.sampler(),
	//		nullptr,
	//		nullptr,
	//};

	///// RenderPass: ClearTargets
	//m_mainTarget.Clear();

	///// RenderPass: SetCameraConstBuffer
	//context()->PSSetConstantBuffers(PASS_CB_CAMERA_PS, 1, m_opaquePass.m_cameraBuffer.GetAddressOf());

	//D3D11_MAPPED_SUBRESOURCE res = {};
	//context()->Map(m_opaquePass.m_cameraBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
	//auto* cbuf = (CameraCBuffer*)res.pData;
	//cbuf->position = camera()->worldPosition();
	//context()->Unmap(m_opaquePass.m_cameraBuffer.Get(), 0);

	///// RenderPass: SetTargets
	//context()->OMSetRenderTargets(8, dxTargets, nullptr);

	///// RenderPass: SetResources
	//context()->VSSetShaderResources(0, SRCount, dxResources);
	//context()->PSSetShaderResources(0, SRCount, dxResources);
	//context()->VSSetSamplers(0, SRCount, dxSamplers);
	//context()->PSSetSamplers(0, SRCount, dxSamplers);

	///// SecondPass: SetLightConstBuffer
	//context()->PSSetConstantBuffers(PASS_CB_LIGHT_PS, 1, m_lightingPass.m_lightCBuffer.GetAddressOf());

	//D3D11_MAPPED_SUBRESOURCE res3 = {};
	//context()->Map(m_lightingPass.m_lightCBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &res3);

	//auto* cbuf2 = (DirectionLightCBuffer*)res3.pData;
	//auto* dirLight = m_game->lighting()->directionLight();
	//cbuf2->uvMatrix = dirLight->uvMatrix();
	//cbuf2->direction = dirLight->forward();
	//cbuf2->color = dirLight->color;
	//cbuf2->intensity = dirLight->intensity;

	//context()->Unmap(m_lightingPass.m_lightCBuffer.Get(), 0);

	///// SecondPass: Draw
	//auto *quad = &m_lightingPass.m_screenQuad;

	///// Material: SetResources
	//ID3D11ShaderResourceView* matResources[] = { quad->deffuse };
	//context()->PSSetShaderResources(PASS_R_MATERIAL_PS, 1, matResources);
	//context()->PSSetSamplers(PASS_R_MATERIAL_PS, 1, quad->m_sampler.GetAddressOf());

	///// Material: SetShader
	//context()->IASetInputLayout(nullptr);
	//context()->VSSetShader(quad->m_shader->vertex.Get(), nullptr, 0);
	//context()->PSSetShader(quad->m_shader->pixel.Get(), nullptr, 0);

	///// Material: SetMaterialConstBuffer
	///// Material: SetRasterizerState
	//context()->RSSetState(GetRastState(CullMode::Back));

	///// SecondPass: DrawShape
	//quad->Draw2();

	///// RenderPass: EndDraw
	//ID3D11RenderTargetView* targets[RTCount] = { nullptr };
	//context()->OMSetRenderTargets(8, targets, nullptr);

	//ID3D11ShaderResourceView* resources[SRCount] = { nullptr };
	//context()->VSSetShaderResources(0, SRCount, resources);
	//context()->PSSetShaderResources(0, SRCount, resources);
}

ID3D11RasterizerState* Render::GetRastState(const Material* material) {
	return GetRastState(material->cullMode, material->fillMode);
}

ID3D11RasterizerState* Render::GetRastState(CullMode cullMode, FillMode fillMode) {

	if (cullMode == CullMode::Back && fillMode == FillMode::Solid)
		return m_ñullSolidBack.Get();

	if (cullMode == CullMode::Front && fillMode == FillMode::Solid)
		return m_ñullSolidFront.Get();

	if (cullMode == CullMode::Back && fillMode == FillMode::Wireframe)
		return m_ñullWireframeBack.Get();

	if (cullMode == CullMode::Front && fillMode == FillMode::Wireframe)
		return m_ñullWireframeFront.Get();

	assert(false);
}