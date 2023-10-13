#include "TextureResource.h"
#include "Render.h"
#include "ImageAsset.h"

void TextureResource::Release() {
	if (m_texture.Get() != nullptr)
		m_texture.ReleaseAndGetAddressOf();
}

TextureResource TextureResource::Create(Render* render, int width, int height, DXGI_FORMAT format) {
	/// format: DXGI_FORMAT_R8G8B8A8_UNORM

	TextureResource res;
	res.render = render;
	res.m_desc = {};

	res.m_desc.Width = width;
	res.m_desc.Height = height;
	res.m_desc.ArraySize = 1;
	res.m_desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	res.m_desc.Usage = D3D11_USAGE_DEFAULT;
	res.m_desc.CPUAccessFlags = 0;
	res.m_desc.Format = format;
	res.m_desc.MipLevels = 1;
	res.m_desc.MiscFlags = 0;
	res.m_desc.SampleDesc.Count = 1;
	res.m_desc.SampleDesc.Quality = 0;

	auto device = render->device();

	auto hres = device->CreateTexture2D(&res.m_desc, nullptr, res.m_texture.GetAddressOf());
	assert(SUCCEEDED(hres));

	return res;
}

TextureResource TextureResource::CreateStagingTexture(Render* render, int width, int height, DXGI_FORMAT format) {
	TextureResource res;
	res.render = render;
	res.m_desc = {};

	res.m_desc.Width = width;
	res.m_desc.Height = height;
	res.m_desc.ArraySize = 1;
	res.m_desc.BindFlags = 0;
	res.m_desc.Usage = D3D11_USAGE_STAGING;
	res.m_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	res.m_desc.Format = format; // DXGI_FORMAT_R32G32_UINT;
	res.m_desc.MipLevels = 1;
	res.m_desc.MiscFlags = 0;
	res.m_desc.SampleDesc.Count = 1;
	res.m_desc.SampleDesc.Quality = 0;

	auto device = render->device();

	auto hres = device->CreateTexture2D(&res.m_desc, nullptr, res.m_texture.GetAddressOf());
	assert(SUCCEEDED(hres));

	return res;
}

TextureResource TextureResource::CreateDepthTexture(Render* render, int width, int height) {
	TextureResource res;
	res.render = render;
	res.m_desc = {};

	res.m_desc.Width = width;
	res.m_desc.Height = height;
	res.m_desc.ArraySize = 1;
	res.m_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	res.m_desc.Usage = D3D11_USAGE_DEFAULT;
	res.m_desc.CPUAccessFlags = 0;
	res.m_desc.Format = DXGI_FORMAT_R32_TYPELESS;
	res.m_desc.MipLevels = 1;
	res.m_desc.MiscFlags = 0;
	res.m_desc.SampleDesc.Count = 1;
	res.m_desc.SampleDesc.Quality = 0;

	auto device = render->device();

	auto hres = device->CreateTexture2D(&res.m_desc, nullptr, res.m_texture.GetAddressOf());
	assert(SUCCEEDED(hres));

	return res;
}

TextureResource TextureResource::CreateFromImage(Render* render, const ImageResource* imageRes) {
	TextureResource res;
	res.render = render;
	res.m_desc = {};

	res.m_desc.Width = imageRes->width;
	res.m_desc.Height = imageRes->height;
	res.m_desc.ArraySize = 1;
	res.m_desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	res.m_desc.Usage = D3D11_USAGE_DEFAULT;
	res.m_desc.CPUAccessFlags = 0;
	res.m_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	res.m_desc.MipLevels = 1;
	res.m_desc.MiscFlags = 0;
	res.m_desc.SampleDesc.Count = 1;
	res.m_desc.SampleDesc.Quality = 0;

	D3D11_SUBRESOURCE_DATA data = {};
	data.pSysMem = imageRes->data;
	data.SysMemPitch = imageRes->lineSize;
	data.SysMemSlicePitch = imageRes->dataSize;

	auto device = render->device();

	auto hres = device->CreateTexture2D(&res.m_desc, &data, res.m_texture.GetAddressOf());
	assert(SUCCEEDED(hres));

	return res;
}