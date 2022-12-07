#pragma once
#include  <utility>

#include "wrl.h_d3d11_alias.h"
#include "SimpleMath.h"

class Render;
class Texture;

class DepthStencil {
public:
	Vector4 clearColor = { 48 / 255.0f, 37 / 255.0f, 60 / 255.0f, 1.0f };

private:
	Render* m_render;

	D3D11_DEPTH_STENCIL_VIEW_DESC m_desc;
	comptr<ID3D11DepthStencilView> m_view;

public:
	ID3D11DepthStencilView* get() { return m_view.Get(); }
	const D3D11_DEPTH_STENCIL_VIEW_DESC& desc() { return m_desc; }

	DepthStencil() {}

	DepthStencil(DepthStencil&& other) noexcept {
		clearColor = other.clearColor;

		m_render = other.m_render;
		m_view = std::move(other.m_view);
	}

	DepthStencil& operator=(DepthStencil&& other) noexcept {
		if (this == &other)
			return *this;

		clearColor = other.clearColor;

		m_render = other.m_render;
		m_view = std::move(other.m_view);

		return *this;
	}

	void Clear();

	static DepthStencil Create(Texture* texture);
	static DepthStencil Create(Render* render, ID3D11Texture2D* texture);

};
