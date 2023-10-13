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
#include "Scene.h"
#include "Actor.h"
#include "Lighting.h"

#include "Refs.h"
#include "RenderPass.h"
#include "OldPass.h"
#include "ILightSource.h"
#include "IShadowCaster.h"

#include "DirectionLight.h"
#include "CameraComponent.h"
#include "MeshComponent.h"

#include "ShaderAsset.h"
#include "RenderTarget.h"
#include "MaterialResource.h"
#include "MaterialAlias.h"
#include "RenderPassUI.h"
//#include "Mesh.h"


#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

const std::string Render::shadowPassName = "Shadow Pass";
const std::string Render::opaquePassName = "Opaque Pass";
const std::string Render::lightingPassName = "Lighting Pass";
const std::string Render::screenSpacePassName = "ScreenSpace Pass";


void Render::Init(Game* game, Window* window) { 
	m_game = game;

	m_device.Create(window->GetHWindow(), window->GetWidth(), window->GetHeight());

	CD3D11_RASTERIZER_DESC rastDesc = {};

	rastDesc.FillMode = D3D11_FILL_SOLID;
	rastDesc.CullMode = D3D11_CULL_BACK;
	device()->CreateRasterizerState(&rastDesc, m_cullSolidBack.GetAddressOf());

	rastDesc.CullMode = D3D11_CULL_FRONT;
	device()->CreateRasterizerState(&rastDesc, m_cullSolidFront.GetAddressOf());

	rastDesc.CullMode = D3D11_CULL_NONE;
	device()->CreateRasterizerState(&rastDesc, m_cullSolidNone.GetAddressOf());

	rastDesc.FillMode = D3D11_FILL_WIREFRAME;
	rastDesc.CullMode = D3D11_CULL_BACK;
	device()->CreateRasterizerState(&rastDesc, m_cullWireframeBack.GetAddressOf());

	rastDesc.CullMode = D3D11_CULL_FRONT;
	device()->CreateRasterizerState(&rastDesc, m_cullWireframeFront.GetAddressOf());

	rastDesc.CullMode = D3D11_CULL_NONE;
	device()->CreateRasterizerState(&rastDesc, m_cullWireframeNone.GetAddressOf());

	onePixelStagingTexUINT = TextureResource::CreateStagingTexture(this, 1, 1, DXGI_FORMAT_R32G32_UINT);
	onePixelStagingTexFLOAT3 = TextureResource::CreateStagingTexture(this, 1, 1, DXGI_FORMAT_R32G32B32A32_FLOAT);
}

void Render::Start() {

}

void Render::Destroy() {

}

void Render::Draw(std::list<Scene*>* scenes) {

	/// Begin Render
	for (auto scene : *scenes) {
		m_scene = scene;
		if (!m_scene->IsDestroyed()) {
			m_renderer = &scene->renderer;

			scene->renderer.Draw();

			m_scene = nullptr;
			m_renderer = nullptr;
		}
	}

	/// Begin Draw
	m_device.BeginDraw();

	/// Draw ImGui
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	/// End Draw
	m_device.EndDraw();

	/// Post Draw
	m_UpdateDeviceSize();

	for (auto scene : *scenes) {
		m_game->PushScene(scene);
		m_scene = scene;
		m_renderer = &scene->renderer;

		scene->renderer.m_UpdateViewportSize();

		m_scene = nullptr;
		m_renderer = nullptr;
		m_game->PopScene();
	}
}

void Render::m_UpdateDeviceSize() {
	auto window = m_game->window();
	if (window->sizeChanged) {
		m_game->window()->sizeChanged = false;

		float width = window->GetWidth();
		float height = window->GetHeight();

		width = width < 10 ? 10 : width;
		height = height < 10 ? 10 : height;

		m_device.Resize(width, height);
	}
}

ID3D11RasterizerState* Render::GetRastState(const MaterialResource* material) {
	return GetRastState(material->cullMode, material->fillMode);
}

ID3D11RasterizerState* Render::GetRastState(CullMode cullMode, FillMode fillMode) {

	if (cullMode == CullMode::Back && fillMode == FillMode::Solid)
		return m_cullSolidBack.Get();

	if (cullMode == CullMode::Front && fillMode == FillMode::Solid)
		return m_cullSolidFront.Get();

	if (cullMode == CullMode::None && fillMode == FillMode::Solid)
		return m_cullSolidNone.Get();

	if (cullMode == CullMode::Back && fillMode == FillMode::Wireframe)
		return m_cullWireframeBack.Get();

	if (cullMode == CullMode::Front && fillMode == FillMode::Wireframe)
		return m_cullWireframeFront.Get();

	if (cullMode == CullMode::None && fillMode == FillMode::Wireframe)
		return m_cullWireframeNone.Get();

	assert(false);
}

UINT Render::GetActorIdInViewport(Scene* scene, const Vector2& vpos)
{
	if (vpos.x < 0 || vpos.x > 1 || vpos.y < 0 || vpos.y > 1)
		return 0;

	auto idsTexture = scene->renderer.idsTexture();
	if (idsTexture == nullptr)
		return 0;

	auto pixel = vpos * scene->renderer.viewportSize();

	D3D11_BOX srcBox;
	srcBox.left = pixel.x;
	srcBox.right = pixel.x + 1;
	srcBox.bottom = pixel.y + 1;
	srcBox.top = pixel.y;
	srcBox.front = 0;
	srcBox.back = 1;

	context()->CopySubresourceRegion(onePixelStagingTexUINT.get(), 0, 0, 0, 0, idsTexture->get(), 0, &srcBox);

	D3D11_MAPPED_SUBRESOURCE MappedSubresource;
	auto hres = context()->Map(onePixelStagingTexUINT.get(), 0, D3D11_MAP_READ, 0, &MappedSubresource);
	assert(SUCCEEDED(hres));

	auto pPixels = (uint32_t*)MappedSubresource.pData;

	context()->Unmap(onePixelStagingTexUINT.get(), 0);

	return (UINT)*pPixels;
}

Vector3 Render::GetWposInViewport(Scene* scene, const Vector2& vpos)
{
	if (vpos.x < 0 || vpos.x > 1 || vpos.y < 0 || vpos.y > 1)
		return Vector3::Zero;

	auto idsTexture = scene->renderer.wposTexture();
	if (idsTexture == nullptr)
		return Vector3::Zero;

	auto pixel = vpos * scene->renderer.viewportSize();

	D3D11_BOX srcBox;
	srcBox.left = pixel.x;
	srcBox.right = pixel.x + 1;
	srcBox.bottom = pixel.y + 1;
	srcBox.top = pixel.y;
	srcBox.front = 0;
	srcBox.back = 1;

	context()->CopySubresourceRegion(onePixelStagingTexFLOAT3.get(), 0, 0, 0, 0, idsTexture->get(), 0, &srcBox);

	D3D11_MAPPED_SUBRESOURCE MappedSubresource;
	auto hres = context()->Map(onePixelStagingTexFLOAT3.get(), 0, D3D11_MAP_READ, 0, &MappedSubresource);
	assert(SUCCEEDED(hres));

	auto pPixels = (Vector3*)MappedSubresource.pData;

	context()->Unmap(onePixelStagingTexFLOAT3.get(), 0);

	return *pPixels;
}
