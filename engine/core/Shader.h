#pragma once

#include "wrl.h_d3d11_alias.h"
#include "FileSystem.h"

class Shader {
public:
	fs::path path;

	comptr<ID3DBlob> vertexBC;
	comptr<ID3DBlob> pixelBC;
	comptr<ID3D11VertexShader> vertex;
	comptr<ID3D11PixelShader> pixel;
	comptr<ID3D11InputLayout> layout;
};

