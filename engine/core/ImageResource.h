#pragma once
#include <filesystem>

#include "wrl.h_d3d11_alias.h"

namespace fs = std::filesystem;


class ImageResource {
private:
	static comptr<IWICImagingFactory2> m_imageFactory;

public:
	size_t width = 0;
	size_t height = 0;
	size_t lineSize = 0;
	size_t dataSize = 0;

	BYTE* data = nullptr;
	
public:
	ImageResource() {};

	ImageResource(ImageResource&& other) noexcept {
		*this = std::move(other);
	}

	ImageResource& operator=(ImageResource&& other) noexcept {
		if (this == &other)
			return *this;

		width = other.width;
		height = other.height;
		lineSize = other.lineSize;
		dataSize = other.dataSize;
		data = other.data;

		other.data = nullptr;

		return *this;
	}
	
	void Init(int width, int height);
	void Release();

public:
	static void InitImageFactory();
	static ImageResource CreateFromFile(const fs::path& path);
};
