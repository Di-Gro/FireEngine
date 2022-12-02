#include "Render.h"

#include <windows.h>
#include <WinUser.h>
#include <wrl.h>
#include <d3d.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <list>

#include "Game.h"
#include "RenderTarget.h"
#include "DirectionLight.h"
#include "ImageComponent.h"
#include "CameraComponent.h"
#include "Refs.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")


void Render::Init(Game* game, Window* window) {
	m_game = game;

	m_device.Create(window->GetHWindow(), window->GetWidth(), window->GetHeight());
	m_mainRT.Init(m_game, window->GetWidth(), window->GetHeight());	
}

void Render::Start() {
	m_screenQuad = m_game->CreateActor("Screen Quad")->AddComponent<ImageComponent>();
	m_screenQuad->shaderPath = "../../data/engine/shaders/screen_quad.hlsl";
	m_screenQuad->SetImage(m_mainRT.renderTexture());
	m_screenQuad->visibility(false);
}

void Render::Draw() {

	RenderTarget* rt = nullptr;

	// Directional light
	m_camera = m_game->lighting()->directionLight()->camera();
	m_device.Clear();
	
	rt = &m_game->lighting()->directionLight()->shadowRT;
	rt->PrepareRender();

	m_Draw(); // m_ForEachGameObject(&Render::m_Draw);
	
	rt->FinishRender();

	// Opaque
	m_camera = m_game->mainCamera();
	m_device.Clear();
	
	rt = &m_mainRT;
	rt->PrepareRender();

	m_Draw(); // m_ForEachGameObject(&Render::m_Draw);
	m_DrawUI(); // m_ForEachGameObject(&Render::m_DrawUI);

	rt->FinishRender();

	// Quad 
	m_camera = m_game->mainCamera();
	m_device.Clear();
	m_device.PrepareFrame();

	m_screenQuad->visibility(true);
	m_screenQuad->OnDrawUI();
	m_screenQuad->visibility(false);

	m_device.EndFrame();

}

std::list<Component*>::iterator Render::SubscribeForDrawin(Component* component) {
	return m_drawers.insert(m_drawers.end(), component);
}

void Render::UnSubscribeFromDrawin(std::list<Component*>::iterator handle) {
	m_drawers.erase(handle);
}

std::list<Component*>::iterator Render::SubscribeForUIDrawin(Component* component) {
	return m_uiDrawers.insert(m_uiDrawers.end(), component);
}

void Render::UnSubscribeFromUIDrawin(std::list<Component*>::iterator handle) {
	m_uiDrawers.erase(handle);
}

//void Render::m_Draw(Actor* gameObject) {
//	if (!gameObject->IsDestroyed())
//		gameObject->f_Draw();
//}
//
//void Render::m_DrawUI(Actor* gameObject) {
//	if (!gameObject->IsDestroyed())
//		gameObject->f_DrawUI();
//}

void Render::m_Draw() {
	for (auto* component : m_drawers) {
		if (!component->IsDestroyed())
			component->OnDraw();

		if (m_camera->drawDebug)
			component->OnDrawDebug();
	}
}

void Render::m_DrawUI() {
	for (auto* component : m_uiDrawers) {
		if (!component->IsDestroyed())
			component->OnDraw();
	}
}

//void Render::m_ForEachGameObject(void (Render::*func) (Actor*)) {
//	for (auto it = m_game->BeginActor(); it != m_game->EndActor(); it++) {
//		(this->*func)(*it);
//	}
//}

void Render::Clear() {
	m_device.Clear();
} 

void Render::PrepareFrame() {
	m_device.PrepareFrame();
}

void Render::EndFrame() {
	m_device.EndFrame();
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