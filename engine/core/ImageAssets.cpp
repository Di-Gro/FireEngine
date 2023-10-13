#include "ImageAssets.h"
#include <cassert>
#include <fstream>
#include <iostream>

#include "Game.h"
#include "Assets.h"
#include "ImageAsset.h"

const ImageAsset* ImageAsset::Default = nullptr;

const fs::path ImageAssets::RUNTIME_IMG_2X2_RGBA_1111 = "runtime:/generated/image/2x2/rgba/1111";
const fs::path ImageAssets::RUNTIME_IMG_2X2_RGBA_0001 = "runtime:/generated/image/2x2/rgba/0001"; 
const fs::path ImageAssets::RUNTIME_IMG_2X2_RGBA_1001 = "runtime:/generated/image/2x2/rgba/1001";


size_t ImageAssets::GetHash(const fs::path& path) {
	return std::hash<std::string>()(path.string());
}

void ImageAssets::Init() {
	m_GenerateRuntimeImages();
}

ImageAssets::~ImageAssets() {
	for (auto& pair : m_images) {
		auto* image = pair.second;
		image->Release();
		delete image;
	}
}

void ImageAssets::Load(const fs::path& path) {
	m_Load(GetHash(path), path);
}

void ImageAssets::m_Load(size_t hash, const fs::path& path) {
	if (m_images.count(hash) > 0)
		return;

	if (!fs::exists(path)) {
		auto text = "ImageAssets: Image file not exist (path:" + path.string() + ")";
		throw std::exception(text.c_str());
	}
	auto* image = new ImageAsset();
	image->resource = ImageResource::CreateFromFile(path);
	m_images.insert({ hash, image });
}

const ImageAsset* ImageAssets::Get(const fs::path& path) {
	auto hash = GetHash(path);

	m_Load(hash, path);
	return m_images.at(hash);
}

void ImageAssets::m_GenerateRuntimeImages() {
	// RUNTIME_IMG_2X2_RGBA_1111
	auto *image = new ImageAsset();
	image->resource.Init(2, 2);

	std::fill(image->resource.data, image->resource.data + image->resource.dataSize, 255);

	auto hash = GetHash(RUNTIME_IMG_2X2_RGBA_1111);
	m_images.insert({ hash, image });
		
	// RUNTIME_IMG_2X2_RGBA_0001
	image = new ImageAsset();
	image->resource.Init(2, 2);

	for (int i = 0; i < image->resource.dataSize; i += 4) {
		image->resource.data[i] = 0;
		image->resource.data[i + 1] = 0;
		image->resource.data[i + 2] = 0;
		image->resource.data[i + 3] = 1;
	}

	hash = GetHash(RUNTIME_IMG_2X2_RGBA_0001);
	m_images.insert({ hash, image });

	// RUNTIME_IMG_2X2_RGBA_1001
	image = new ImageAsset();
	image->resource.Init(2, 2);

	for (int i = 0; i < image->resource.dataSize; i += 4) {
		image->resource.data[i] = 1;
		image->resource.data[i + 1] = 0;
		image->resource.data[i + 2] = 0;
		image->resource.data[i + 3] = 1;
	}

	hash = GetHash(RUNTIME_IMG_2X2_RGBA_1001);
	m_images.insert({ hash, image });
}


