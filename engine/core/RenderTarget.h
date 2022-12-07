#pragma once
#include  <utility>

#include "wrl.h_d3d11_alias.h"
#include "SimpleMath.h"

class Render;
class Texture;

class RenderTarget {
public:
	Vector4 clearColor = { 48 / 255.0f, 37 / 255.0f, 60 / 255.0f, 1.0f };

private:
	Render* m_render = nullptr;

	comptr<ID3D11RenderTargetView> m_view;
	
public:

	ID3D11RenderTargetView* get() { return m_view.Get(); }
	
	RenderTarget() {}

	RenderTarget(RenderTarget&& other) noexcept {
		clearColor = other.clearColor;

		m_render = other.m_render;
		m_view = std::move(other.m_view);
	}

	RenderTarget& operator=(RenderTarget&& other) noexcept {
		if (this == &other)
			return *this;

		clearColor = other.clearColor;

		m_render = other.m_render;
		m_view = std::move(other.m_view);

		return *this;
	}

	void Clear();

	static RenderTarget Create(Texture* texture);

};
