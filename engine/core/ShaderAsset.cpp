#include "ShaderAsset.h"

#include <iostream>

#include <windows.h>
#include <WinUser.h>
#include <wrl.h>
#include <d3d.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

#include "Render.h"


void ShaderAsset::Init(Render* render) {
	m_render = render;
}

void ShaderAsset::CompileShader(const fs::path& path) {
	auto hash = GetShaderHash(path);

	if (HasShader(hash))
		return;

	m_shaders.insert(std::make_pair(hash, Shader()));
	Shader& shader = m_shaders.at(hash);

	if (!m_CompileShader(path, &shader))
		std::exit(1);
}

void ShaderAsset::RecompileShaders() {
	for (auto& key_value : m_shaders) {
		Shader next;
		Shader& current = m_shaders.at(key_value.first);

		std::cout << "Recompile: " << current.path << std::endl;
		if (m_CompileShader(current.path, &next))
			current = next;
	}
}

bool ShaderAsset::m_CompileShader(const fs::path& path, Shader* shader) {

	shader->path = path;
	shader->vertexBC = nullptr;
	shader->pixelBC = nullptr;
	shader->vertex = nullptr;
	shader->pixel = nullptr;
	shader->layout = nullptr;

	ID3DBlob* errorVertexCode = nullptr;
	auto hres = D3DCompileFromFile(path.wstring().c_str(),
		nullptr /*macros*/,
		nullptr /*include*/,
		"VSMain",
		"vs_5_0",
		D3DCOMPILE_PACK_MATRIX_ROW_MAJOR, // D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		shader->vertexBC.GetAddressOf(),
		&errorVertexCode);

	if (FAILED(hres)) {
		if (errorVertexCode) {
			char* compileErrors = (char*)(errorVertexCode->GetBufferPointer());
			std::cout << compileErrors << std::endl;
		}
		else {
			std::wcout << "Missing Shader File: " << std::wstring(path) << std::endl;
		}
		return false;
	}

	//D3D_SHADER_MACRO Shader_Macros[] = { "TEST", "1", "TCOLOR", "float4(0.0f, 1.0f, 0.0f, 1.0f)", nullptr, nullptr };

	//bool errorX3501 = false; // Нет шейдера с таким именем
	ID3DBlob* errorPixelCode = nullptr;
	hres = D3DCompileFromFile(
		path.wstring().c_str(),
		nullptr, //Shader_Macros /*macros*/,
		nullptr /*include*/,
		"PSMain",
		"ps_5_0",
		D3DCOMPILE_PACK_MATRIX_ROW_MAJOR, // D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		shader->pixelBC.GetAddressOf(),
		&errorPixelCode);

	if (FAILED(hres)) {
		if (errorPixelCode) {
			std::string errors((char*)(errorPixelCode->GetBufferPointer()));
			//errorX3501 = errors.find("error X3501:") != std::string::npos;
			std::cout << errors << std::endl;
		}
		else {
			std::wcout << "Pixel Shader not loaded: " << std::wstring(path) << std::endl;
		}
		//if (!errorX3501)
		return false;
	}

	m_render->device()->CreateVertexShader(
		shader->vertexBC->GetBufferPointer(),
		shader->vertexBC->GetBufferSize(),
		nullptr,
		shader->vertex.GetAddressOf());

	//if (!errorX3501) {
	m_render->device()->CreatePixelShader(
		shader->pixelBC->GetBufferPointer(),
		shader->pixelBC->GetBufferSize(),
		nullptr,
		shader->pixel.GetAddressOf());
	//}

	D3D11_INPUT_ELEMENT_DESC inputElements[] = {
		D3D11_INPUT_ELEMENT_DESC {
			"POSITION",
			0,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			0,
			0 ,
			D3D11_INPUT_PER_VERTEX_DATA,
			0},
		D3D11_INPUT_ELEMENT_DESC {
			"COLOR",
			0,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			0,
			D3D11_APPEND_ALIGNED_ELEMENT ,
			D3D11_INPUT_PER_VERTEX_DATA,
			0},
		D3D11_INPUT_ELEMENT_DESC {
			"NORMAL",
			0,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			0,
			D3D11_APPEND_ALIGNED_ELEMENT ,
			D3D11_INPUT_PER_VERTEX_DATA,
			0},
		D3D11_INPUT_ELEMENT_DESC {
			"TEXCOORD",
			0,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			0,
			D3D11_APPEND_ALIGNED_ELEMENT,
			D3D11_INPUT_PER_VERTEX_DATA,
			0}
	};

	m_render->device()->CreateInputLayout(
		inputElements,
		4,
		shader->vertexBC->GetBufferPointer(),
		shader->vertexBC->GetBufferSize(),
		shader->layout.GetAddressOf());

	return true;
}