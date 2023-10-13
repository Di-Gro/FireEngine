#pragma once

#include "wrl.h_d3d11_alias.h"
#include "CSBridge.h"

#include "IAsset.h"
#include "ImageResource.h"

class ImageAsset : public IAsset {
public:
	static const ImageAsset* Default;

public:
	ImageResource resource;

public:
	void Release() override;
};

PUSH_ASSET(ImageAsset);
FUNC(ImageAsset, Init, void)(CppRef gameRef, CppRef imgRef, const char* path, int& width, int& height);