#pragma once

#include <string>
#include <unordered_map>
#include <filesystem>

#include "wrl.h_d3d11_alias.h"
#include "CSBridge.h"

#include "IAsset.h"
#include "ImageResource.h"

namespace fs = std::filesystem;

class ImageAsset;

class ImageAssets {
public:
	static const fs::path RUNTIME_IMG_2X2_RGBA_1111;
	static const fs::path RUNTIME_IMG_2X2_RGBA_0001;
	static const fs::path RUNTIME_IMG_2X2_RGBA_1001;

private:
	std::unordered_map<size_t, ImageAsset*> m_images;

public:
	~ImageAssets();

	void Init();
	void Load(const fs::path& path);
	
	const ImageAsset* Get(const fs::path& path);

	static size_t GetHash(const fs::path& path);

private:
	void m_Load(size_t hash, const fs::path& path);
	void m_GenerateRuntimeImages();

};
