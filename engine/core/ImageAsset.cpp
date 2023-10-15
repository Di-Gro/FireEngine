#include "ImageAsset.h"
#include "Game.h"
#include "Assets.h"

const ImageAsset* ImageAsset::Default = nullptr;

void ImageAsset::Release() {
	resource.Release();
}

DEF_PUSH_ASSET(ImageAsset);

DEF_FUNC(ImageAsset, Init, void)(CppRef gameRef, CppRef imgRef, const char* path, int& width, int& height) {
	auto* game = CppRefs::ThrowPointer<Game>(gameRef);
	auto* image = CppRefs::ThrowPointer<ImageAsset>(imgRef);

	image->resource = ImageResource::CreateFromFile(path);

	width = image->resource.width;
	height = image->resource.height;
}
