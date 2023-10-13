#pragma once
#include <utility>
#include <string>

#include "wrl.h_d3d11_alias.h"

class Render;
class ImageResource;

class TextureResource {
public:
	Render* render = nullptr;

private:
	D3D11_TEXTURE2D_DESC m_desc = {};
	comptr<ID3D11Texture2D> m_texture;

public:
	TextureResource() {};

	TextureResource(TextureResource&& other) noexcept {
		*this = std::move(other);
	}

	TextureResource& operator=(TextureResource&& other) noexcept {
		if (this == &other)
			return *this;

		m_desc = other.m_desc;
		m_texture = std::move(other.m_texture);
		render = other.render;

		return *this;
	}

	bool IsEmpty() { return m_texture.Get() == nullptr; }
	ID3D11Texture2D* get() { return m_texture.Get(); }
	const D3D11_TEXTURE2D_DESC& desc() { return m_desc; }

	void Release();

	static TextureResource Create(Render* render, int width, int height, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM);
	static TextureResource CreateStagingTexture(Render* render, int width, int height, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM);

	static TextureResource CreateDepthTexture(Render* render, int width, int height);
	static TextureResource CreateFromImage(Render* render, const ImageResource* image);

};