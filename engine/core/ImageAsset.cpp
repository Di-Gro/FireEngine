#include "ImageAsset.h"
#include <cassert>
#include <fstream>
#include <iostream>

#include "Game.h"

const fs::path ImageAsset::RUNTIME_IMG_2X2_RGBA_1111 = "runtime:/generated/image/2x2/rgba/1111";
const fs::path ImageAsset::RUNTIME_IMG_2X2_RGBA_0001 = "runtime:/generated/image/2x2/rgba/0001"; 
const fs::path ImageAsset::RUNTIME_IMG_2X2_RGBA_1001 = "runtime:/generated/image/2x2/rgba/1001";

void Image::Init(int width, int height) {
	this->width = width;
	this->height = height;
	lineSize = this->width * 4;
	dataSize = lineSize * this->height;
	data = new BYTE[dataSize];
}

void Image::Release() {
	if (data != nullptr)
		delete[] data;
}

size_t ImageAsset::GetHash(const fs::path& path) {
	return std::hash<std::string>()(path.string());
}

void ImageAsset::Init() {
	auto hres = CoInitialize(NULL);
	assert(SUCCEEDED(hres));

	hres = CoCreateInstance(CLSID_WICImagingFactory2, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(m_imageFactory.GetAddressOf()));
	assert(SUCCEEDED(hres));

	m_GenerateRuntimeImages();
}

ImageAsset::~ImageAsset() {
	for (auto& pair : m_images) {
		auto* image = pair.second;
		image->Release();
		delete image;
	}
}

void ImageAsset::Load(const fs::path& path) {
	m_Load(GetHash(path), path);
}

void ImageAsset::m_Load(size_t hash, const fs::path& path) {
	if (m_images.count(hash) > 0)
		return;

	if (!fs::exists(path)) {
		auto text = "ImageAsset: Image file not exist (path:" + path.string() + ")";
		throw std::exception(text.c_str());
	}
	auto* image = m_CreateImage(path);
	m_images.insert({ hash, image });
}

const Image* ImageAsset::Get(const fs::path& path) {
	auto hash = GetHash(path);

	m_Load(hash, path);
	return m_images.at(hash);
}

Image* ImageAsset::m_CreateImage(const fs::path& path) {
	comptr<IWICBitmapDecoder> decoder;
	comptr<IWICBitmapFrameDecode> frame;
	comptr<IWICFormatConverter> converter;

	auto hres = m_imageFactory->CreateDecoderFromFilename(path.wstring().c_str(), 0, GENERIC_READ, WICDecodeMetadataCacheOnDemand, decoder.GetAddressOf());
	assert(SUCCEEDED(hres));

	hres = decoder->GetFrame(0, frame.GetAddressOf());
	assert(SUCCEEDED(hres));

	hres = m_imageFactory->CreateFormatConverter(converter.GetAddressOf());
	assert(SUCCEEDED(hres));

	hres = converter->Initialize(frame.Get(), GUID_WICPixelFormat32bppPRGBA, WICBitmapDitherTypeNone, nullptr, 0, WICBitmapPaletteTypeCustom);
	assert(SUCCEEDED(hres));

	UINT w, h;
	converter->GetSize(&w, &h);

	auto* image = new Image();
	image->Init(w, h);

	hres = converter->CopyPixels(nullptr, image->lineSize, image->dataSize, image->data);
	assert(SUCCEEDED(hres));

	return image;
}

void ImageAsset::m_GenerateRuntimeImages() {
	// RUNTIME_IMG_2X2_RGBA_1111
	auto *image = new Image();
	image->Init(2, 2);

	std::fill(image->data, image->data + image->dataSize, 255);

	auto hash = GetHash(RUNTIME_IMG_2X2_RGBA_1111);
	m_images.insert({ hash, image });
		
	// RUNTIME_IMG_2X2_RGBA_0001
	image = new Image();
	image->Init(2, 2);

	for (int i = 0; i < image->dataSize; i += 4) {
		image->data[i] = 0;
		image->data[i + 1] = 0;
		image->data[i + 2] = 0;
		image->data[i + 3] = 1;
	}

	hash = GetHash(RUNTIME_IMG_2X2_RGBA_0001);
	m_images.insert({ hash, image });

	// RUNTIME_IMG_2X2_RGBA_1001
	image = new Image();
	image->Init(2, 2);

	for (int i = 0; i < image->dataSize; i += 4) {
		image->data[i] = 1;
		image->data[i + 1] = 0;
		image->data[i + 2] = 0;
		image->data[i + 3] = 1;
	}

	hash = GetHash(RUNTIME_IMG_2X2_RGBA_1001);
	m_images.insert({ hash, image });
}

DEF_FUNC(ImageAsset, Load, CppRef)(CppRef gameRef, const char* path, int& width, int& height) {
	auto *game = CppRefs::ThrowPointer<Game>(gameRef);

	auto* image = game->imageAsset()->Get(path);
	auto assetRef = CppRefs::GetRef((void*)image);
	if (assetRef == 0)
		assetRef = CppRefs::Create((void*)image).cppRef();

	width = image->width;
	height = image->height;

	return assetRef;
}

