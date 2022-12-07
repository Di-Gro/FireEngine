#pragma once
#include <utility>
#include <string>

#include "wrl.h_d3d11_alias.h"


class Render;
class Image;

class Texture {
private:
	D3D11_TEXTURE2D_DESC m_desc;
	comptr<ID3D11Texture2D> m_texture;

public:
	std::string name;
	Render* render = nullptr;

	bool IsEmpty() { return m_texture.Get() == nullptr; }
	ID3D11Texture2D* get() { return m_texture.Get(); }
	const D3D11_TEXTURE2D_DESC& desc() { return m_desc; }

	Texture() {};

	Texture(Texture&& other) noexcept {
		m_desc = other.m_desc;
		m_texture = std::move(other.m_texture);
		name = std::move(other.name);
		render = other.render;
	}

	Texture& operator=(Texture&& other) noexcept {
		if (this == &other)
			return *this;

		m_desc = other.m_desc;
		m_texture = std::move(other.m_texture);
		name = std::move(other.name);
		render = other.render;
		
		return *this;
	}

	static Texture Create(Render* render, int width, int height);
	static Texture CreateDepthTexture(Render* render, int width, int height);
	static Texture CreateFromImage(Render* render, const Image* image);
};