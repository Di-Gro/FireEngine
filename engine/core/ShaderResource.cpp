#include "ShaderResource.h"

#include "Render.h"
#include "Texture.h"


ShaderResource ShaderResource::Create(Texture* texture) {
	ShaderResource res;
	res.m_render = texture->render;
	auto device = res.m_render->device();

	// Depth Texture
	if(texture->desc().Format == DXGI_FORMAT_R32_TYPELESS) 
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC depthSRVDesc = {};
		depthSRVDesc.Format = DXGI_FORMAT_R32_FLOAT;
		depthSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		depthSRVDesc.Texture2D.MipLevels = 1;
		depthSRVDesc.Texture2D.MostDetailedMip = 0;

		res.samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		res.samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		res.samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		res.samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
		res.samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
		res.samplerDesc.BorderColor[0] = 1.0f;
		res.samplerDesc.BorderColor[1] = 0.0f;
		res.samplerDesc.BorderColor[2] = 0.0f;
		res.samplerDesc.BorderColor[3] = 1.0f;
		res.samplerDesc.MaxLOD = INT_MAX;

		auto hres = device->CreateShaderResourceView(texture->get(), &depthSRVDesc, res.m_view.GetAddressOf());
		assert(SUCCEEDED(hres));

		hres = device->CreateSamplerState(&res.samplerDesc, res.m_sampler.GetAddressOf());
		assert(SUCCEEDED(hres));
	}
	else {
		D3D11_SHADER_RESOURCE_VIEW_DESC renderSRVDesc = {};
		renderSRVDesc.Format = texture->desc().Format;
		renderSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		renderSRVDesc.Texture2D.MipLevels = 1;
		renderSRVDesc.Texture2D.MostDetailedMip = 0;

		res.samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		res.samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		res.samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		res.samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		res.samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		res.samplerDesc.BorderColor[0] = 1.0f;
		res.samplerDesc.BorderColor[1] = 0.0f;
		res.samplerDesc.BorderColor[2] = 0.0f;
		res.samplerDesc.BorderColor[3] = 1.0f;
		res.samplerDesc.MaxLOD = INT_MAX;

		auto hres = device->CreateShaderResourceView(texture->get(), &renderSRVDesc, res.m_view.GetAddressOf());
		assert(SUCCEEDED(hres));

		hres = device->CreateSamplerState(&res.samplerDesc, res.m_sampler.GetAddressOf());
		assert(SUCCEEDED(hres));
	}
	return res;
}

void ShaderResource::UpdateSampler() {
	if (m_sampler.Get() != nullptr)
		m_sampler.ReleaseAndGetAddressOf();

	auto hres = m_render->device()->CreateSamplerState(&samplerDesc, m_sampler.GetAddressOf());
	assert(SUCCEEDED(hres));
}