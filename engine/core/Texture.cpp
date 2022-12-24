#include "Texture.h"

#include "Render.h"
#include "MeshAsset.h"


Texture Texture::Create(Render* render, int width, int height, DXGI_FORMAT format) {
	/// format: DXGI_FORMAT_R8G8B8A8_UNORM

	Texture res;
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

//Texture Texture::CreateActorIdTexture(Render* render, int width, int height) {
//	Texture res;
//	res.render = render;
//	res.m_desc = {};
//
//	res.m_desc.Width = width;
//	res.m_desc.Height = height;
//	res.m_desc.ArraySize = 1;
//	res.m_desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
//	res.m_desc.Usage = D3D11_USAGE_STAGING;
//	res.m_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
//	res.m_desc.Format = DXGI_FORMAT_R32G32_UINT;
//	res.m_desc.MipLevels = 1;
//	res.m_desc.MiscFlags = 0;
//	res.m_desc.SampleDesc.Count = 1;
//	res.m_desc.SampleDesc.Quality = 0;
//
//	auto device = render->device();
//
//	auto hres = device->CreateTexture2D(&res.m_desc, nullptr, res.m_texture.GetAddressOf());
//	assert(SUCCEEDED(hres));
//	
//	return res;
//}

Texture Texture::CreateStagingTexture(Render* render, int width, int height, DXGI_FORMAT format) {
	Texture res;
	res.render = render;
	res.m_desc = {};

	res.m_desc.Width = width;
	res.m_desc.Height = height;
	res.m_desc.ArraySize = 1;
	res.m_desc.BindFlags = 0;
	res.m_desc.Usage = D3D11_USAGE_STAGING;
	res.m_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	res.m_desc.Format = DXGI_FORMAT_R32G32_UINT;
	res.m_desc.MipLevels = 1;
	res.m_desc.MiscFlags = 0;
	res.m_desc.SampleDesc.Count = 1;
	res.m_desc.SampleDesc.Quality = 0;

	auto device = render->device();

	auto hres = device->CreateTexture2D(&res.m_desc, nullptr, res.m_texture.GetAddressOf());
	assert(SUCCEEDED(hres));

	return res;
}

Texture Texture::CreateDepthTexture(Render* render, int width, int height) {
	Texture res;
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

Texture Texture::CreateFromImage(Render* render, const Image* image) {
	Texture res;
	res.render = render;
	res.m_desc = {};

	res.m_desc.Width = image->width;
	res.m_desc.Height = image->height;
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
	data.pSysMem = image->data;
	data.SysMemPitch = image->lineSize;
	data.SysMemSlicePitch = image->dataSize;

	auto device = render->device();

	auto hres = device->CreateTexture2D(&res.m_desc, &data, res.m_texture.GetAddressOf());
	assert(SUCCEEDED(hres));

	return res;
}