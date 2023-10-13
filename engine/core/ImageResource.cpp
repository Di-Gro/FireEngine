#include "ImageResource.h"

#include <cassert>
#include <string>
#include <unordered_map>
#include <filesystem>

#include "wrl.h_d3d11_alias.h"
#include "CSBridge.h"

#include "IAsset.h"
#include "ImageResource.h"

namespace fs = std::filesystem;

comptr<IWICImagingFactory2> ImageResource::m_imageFactory = nullptr;

void ImageResource::InitImageFactory() {
	if (m_imageFactory.Get() != nullptr)
		return;

	auto hres = CoInitialize(NULL);
	assert(SUCCEEDED(hres));

	hres = CoCreateInstance(CLSID_WICImagingFactory2, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(m_imageFactory.GetAddressOf()));
	assert(SUCCEEDED(hres));
}

void ImageResource::Init(int width, int height) {
	this->width = width;
	this->height = height;
	lineSize = this->width * 4;
	dataSize = lineSize * this->height;
	data = new BYTE[dataSize];
}

void ImageResource::Release() {
	if (data != nullptr)
		delete[] data;
}

ImageResource ImageResource::CreateFromFile(const fs::path& path) {
	InitImageFactory();

	ImageResource res;

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

	res.Init(w, h);

	hres = converter->CopyPixels(nullptr, res.lineSize, res.dataSize, res.data);
	assert(SUCCEEDED(hres));

	return res;
}