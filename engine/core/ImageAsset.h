#pragma once

#include <string>
#include <unordered_map>

#include "wrl.h_d3d11_alias.h"
#include "FileSystem.h"

class Image {
public:
	size_t width = 0;
	size_t height = 0;
	size_t lineSize = 0;
	size_t dataSize = 0;

	BYTE* data = nullptr;

	void Init(int width, int height);
	void Release();
};

class ImageAsset {

public:
	static const fs::path RUNTIME_IMG_2X2_RGBA_1111;
	static const fs::path RUNTIME_IMG_2X2_RGBA_0001;
	static const fs::path RUNTIME_IMG_2X2_RGBA_1001;

private:
	std::unordered_map<size_t, Image*> m_images;

	comptr<IWICImagingFactory2> m_imageFactory;

public:
	~ImageAsset();

	void Init();
	void Load(fs::path path);
	
	const Image* Get(fs::path path);

	static size_t GetHash(fs::path path);

private:
	void m_Load(size_t hash, fs::path path);
	Image* m_CreateImage(fs::path path);
	void m_GenerateRuntimeImages();

};

