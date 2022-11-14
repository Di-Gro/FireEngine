#pragma once

#include <string>
#include <map>

#include "wrl.h_d3d11_alias.h"
#include "FileSystem.h"


class ImageAsset {

public:
	static const fs::path RUNTIME_IMG_2X2_RGBA_1111;
	static const fs::path RUNTIME_IMG_2X2_RGBA_0001;
	static const fs::path RUNTIME_IMG_2X2_RGBA_1001;

public:
	class Image {
	public:
		UINT width = 0;
		UINT height = 0;
		UINT lineSize = 0;
		UINT dataSize = 0;

		BYTE* data = nullptr;

		void Init(int width, int height);
		void Release();
	};

private:
	std::map<UINT, ImageAsset::Image> m_images;

	comptr<IWICImagingFactory2> m_imageFactory;

public:
	~ImageAsset();

	void Init();
	void Load(fs::path path);
	
	const ImageAsset::Image* Get(fs::path path);

	static UINT GetHash(fs::path path);

private:
	void m_Load(UINT hash, fs::path path);
	ImageAsset::Image m_CreateImage(fs::path path);
	void m_GenerateRuntimeImages();

};

