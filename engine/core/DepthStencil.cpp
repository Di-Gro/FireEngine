#include "DepthStencil.h"

#include "Render.h"
#include "Texture.h"


DepthStencil DepthStencil::Create(Texture* texture) {
	DepthStencil res;
	res.m_render = texture->render;

	res.m_desc = {};
	res.m_desc.Format = DXGI_FORMAT_D32_FLOAT;
	res.m_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	res.m_desc.Flags = 0;

	auto device = res.m_render->device();

	auto hres = device->CreateDepthStencilView(texture->get(), &res.m_desc, res.m_view.GetAddressOf());
	assert(SUCCEEDED(hres));

	res.Clear();
	return res;
}

DepthStencil DepthStencil::Create(Render* render, ID3D11Texture2D* texture) {
	DepthStencil res;
	res.m_render = render;

	res.m_desc = {};
	res.m_desc.Format = DXGI_FORMAT_D32_FLOAT;
	res.m_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	res.m_desc.Flags = 0;

	auto device = res.m_render->device();

	auto hres = device->CreateDepthStencilView(texture, &res.m_desc, res.m_view.GetAddressOf());
	assert(SUCCEEDED(hres));

	res.Clear();
	return res;
}

void DepthStencil::Clear() {
	auto context = m_render->context();

	context->ClearDepthStencilView(m_view.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}
