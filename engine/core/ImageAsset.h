#pragma once

#include <string>
#include <unordered_map>
#include <filesystem>

#include "wrl.h_d3d11_alias.h"
#include "CSBridge.h"

#include "IAsset.h"

namespace fs = std::filesystem;

class Image : public IAsset {
public:
	static const Image* Default;

public:
	size_t width = 0;
	size_t height = 0;
	size_t lineSize = 0;
	size_t dataSize = 0;

	BYTE* data = nullptr;

	void Init(int width, int height);
	void Release() override;
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
	void Load(const fs::path& path);
	
	void InitImage(Image* image, const fs::path& path);

	const Image* Get(const fs::path& path);

	static size_t GetHash(const fs::path& path);

private:
	void m_Load(size_t hash, const fs::path& path);
	//Image* m_CreateImage(const fs::path& path);
	void m_GenerateRuntimeImages();

};

PUSH_ASSET(Image);
FUNC(Image, Init, void)(CppRef gameRef, CppRef imgRef, const char* path, int& width, int& height);