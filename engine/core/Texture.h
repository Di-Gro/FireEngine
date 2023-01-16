#pragma once
#include <utility>
#include <string>

#include "wrl.h_d3d11_alias.h"
#include "CSBridge.h"
#include "IAsset.h"

class Render;
class Image;

class Texture : public IAsset {
private:
	D3D11_TEXTURE2D_DESC m_desc;
	comptr<ID3D11Texture2D> m_texture;

public:
	std::string name;
	Render* render = nullptr;
	const Image* image = nullptr;

	bool IsEmpty() { return m_texture.Get() == nullptr; }
	ID3D11Texture2D* get() { return m_texture.Get(); }
	const D3D11_TEXTURE2D_DESC& desc() { return m_desc; }

	Texture() {};

	Texture(Texture&& other) noexcept {
		m_desc = other.m_desc;
		m_texture = std::move(other.m_texture);
		name = std::move(other.name);
		render = other.render;
		image = other.image;
	}

	Texture& operator=(Texture&& other) noexcept {
		if (this == &other)
			return *this;

		m_desc = other.m_desc;
		m_texture = std::move(other.m_texture);
		name = std::move(other.name);
		render = other.render;
		image = other.image;
		
		return *this;
	}

	void Release() override;

	static Texture Create(Render* render, int width, int height, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM);
	//static Texture CreateActorIdTexture(Render* render, int width, int height);
	static Texture CreateStagingTexture(Render* render, int width, int height, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM);

	static Texture CreateDepthTexture(Render* render, int width, int height);
	static Texture CreateFromImage(Render* render, const Image* image);
};

PUSH_ASSET(Texture);

FUNC(Texture, Init, void)(CppRef gameRef, CppRef texRef, UINT width, UINT height);
FUNC(Texture, InitFromImage, void)(CppRef gameRef, CppRef texRef, CppRef imageRef);