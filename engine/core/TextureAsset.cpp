#include "TextureAsset.h"

#include "Render.h"
#include "Assets.h"
#include "MeshAsset.h"
#include "ImageAsset.h"
#include "Game.h"

TextureAsset* TextureAsset::Default = nullptr;


void TextureAsset::Release() {
	resource.Release();
}

DEF_PUSH_ASSET(TextureAsset);

DEF_FUNC(TextureAsset, Init, void)(CppRef gameRef, CppRef texRef, UINT width, UINT height) {
	auto game = CppRefs::ThrowPointer<Game>(gameRef);
	auto texture = CppRefs::ThrowPointer<TextureAsset>(texRef);

	texture->resource = TextureResource::Create(game->render(), width, height);
}

FUNC(TextureAsset, InitFromImage, void)(CppRef gameRef, CppRef texRef, CppRef imageRef) {
	auto game = CppRefs::ThrowPointer<Game>(gameRef);
	auto texture = CppRefs::ThrowPointer<TextureAsset>(texRef);
	auto image = CppRefs::ThrowPointer<ImageAsset>(imageRef);

	texture->image = image;
	texture->resource = TextureResource::CreateFromImage(game->render(), &image->resource);
}