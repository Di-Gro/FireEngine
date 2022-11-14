#include "ImageAsset.h"
#include <cassert>
#include <fstream>
#include <iostream>

const fs::path ImageAsset::RUNTIME_IMG_2X2_RGBA_1111 = "runtime:/generated/image/2x2/rgba/1111";
const fs::path ImageAsset::RUNTIME_IMG_2X2_RGBA_0001 = "runtime:/generated/image/2x2/rgba/0001"; 
const fs::path ImageAsset::RUNTIME_IMG_2X2_RGBA_1001 = "runtime:/generated/image/2x2/rgba/1001";

void ImageAsset::Image::Init(int width, int height) {
	this->width = width;
	this->height = height;
	lineSize = this->width * 4;
	dataSize = lineSize * this->height;
	data = new BYTE[dataSize];
}

void ImageAsset::Image::Release() {
	if (data != nullptr)
		delete[] data;
}

UINT ImageAsset::GetHash(fs::path path) {
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
	for (auto& pair : m_images)
		pair.second.Release();
}

void ImageAsset::Load(fs::path path) {
	m_Load(GetHash(path), path);
}

void ImageAsset::m_Load(UINT hash, fs::path path) {
	if (m_images.count(hash) > 0)
		return;

	Image image = m_CreateImage(path);
	m_images.insert({ hash, image });
}

const ImageAsset::Image* ImageAsset::Get(fs::path path) {
	auto hash = GetHash(path);

	m_Load(hash, path);
	return &m_images.at(hash);
}

ImageAsset::Image ImageAsset::m_CreateImage(fs::path path) {
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

	Image image;
	image.Init(w, h);

	hres = converter->CopyPixels(nullptr, image.lineSize, image.dataSize, image.data);
	assert(SUCCEEDED(hres));

	return image;
}

void ImageAsset::m_GenerateRuntimeImages() {
	Image image;
	image.Init(2, 2);

	std::fill(image.data, image.data + image.dataSize, 255);

	auto hash = GetHash(RUNTIME_IMG_2X2_RGBA_1111);
	m_images.insert({ hash, image });
		
	image.Init(2, 2);

	for (int i = 0; i < image.dataSize; i += 4) {
		image.data[i] = 0;
		image.data[i + 1] = 0;
		image.data[i + 2] = 0;
		image.data[i + 3] = 1;
	}

	hash = GetHash(RUNTIME_IMG_2X2_RGBA_0001);
	m_images.insert({ hash, image });

	image.Init(2, 2);

	for (int i = 0; i < image.dataSize; i += 4) {
		image.data[i] = 1;
		image.data[i + 1] = 0;
		image.data[i + 2] = 0;
		image.data[i + 3] = 1;
	}

	hash = GetHash(RUNTIME_IMG_2X2_RGBA_1001);
	m_images.insert({ hash, image });
}


