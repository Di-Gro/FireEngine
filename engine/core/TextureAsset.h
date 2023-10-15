#pragma once
#include <utility>
#include <string>

#include "wrl.h_d3d11_alias.h"
#include "CSBridge.h"
#include "IAsset.h"
#include "TextureResource.h"

class Render;
class ImageAsset;

class TextureAsset : public IAsset {
public:
	static const TextureAsset* Default;

public:
	std::string name;
	const ImageAsset* image = nullptr;
	TextureResource resource;

public:
	TextureAsset() {};

	TextureAsset(TextureAsset&& other) noexcept {
		*this = std::move(other);
	}

	TextureAsset& operator=(TextureAsset&& other) noexcept {
		if (this == &other)
			return *this;

		name = std::move(other.name);
		image = other.image;
		resource = std::move(other.resource);

		other.image = nullptr;
		
		return *this;
	}

	void Release() override;
};

PUSH_ASSET(TextureAsset);

FUNC(TextureAsset, Init, void)(CppRef gameRef, CppRef texRef, UINT width, UINT height);
FUNC(TextureAsset, InitFromImage, void)(CppRef gameRef, CppRef texRef, CppRef imageRef);