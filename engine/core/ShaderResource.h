#pragma once
#include  <utility>

#include "wrl.h_d3d11_alias.h"

class Render;
class Texture;

class ShaderResource {
public:
	D3D11_SAMPLER_DESC samplerDesc;

private:
	Render* m_render;

	comptr<ID3D11ShaderResourceView> m_view;
	comptr<ID3D11SamplerState> m_sampler;

public:
	ID3D11ShaderResourceView* get() const { return m_view.Get(); }
	ID3D11ShaderResourceView*const* getRef() const { return m_view.GetAddressOf(); }
	ID3D11SamplerState* sampler() const { return m_sampler.Get(); }
	ID3D11SamplerState* const* samplerRef() const { return m_sampler.GetAddressOf(); }

	ShaderResource() {}

	ShaderResource(ShaderResource&& other) {
		samplerDesc = other.samplerDesc;
		m_render = other.m_render;
		m_view = std::move(other.m_view);
		m_sampler = std::move(other.m_sampler);
	}

	ShaderResource& operator=(ShaderResource&& other) noexcept {
		if (this == &other)
			return *this;

		samplerDesc = other.samplerDesc;
		m_render = other.m_render;
		m_view = std::move(other.m_view);
		m_sampler = std::move(other.m_sampler);

		return *this;
	}

	void UpdateSampler();

	static ShaderResource Create(Texture* texture);
	static ShaderResource Create(Render* render, ID3D11Texture2D* texture);
};