#include "RenderTarget.h"
#include "Game.h"
#include "Render.h"
#include "RenderDevice.h"

#include "Render.h"
#include "Texture.h"


RenderTarget RenderTarget::Create(Texture* texture) {
	RenderTarget res;
	res.m_render = texture->render;

	auto device = res.m_render->device();

	auto hres = device->CreateRenderTargetView(texture->get(), nullptr, res.m_view.GetAddressOf());
	assert(SUCCEEDED(hres));

	res.Clear();
	return res;
}

void RenderTarget::Clear() {
	auto context = m_render->context();

	context->ClearRenderTargetView(m_view.Get(), (const FLOAT*)&clearColor);
}
