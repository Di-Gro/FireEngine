#pragma once
#include "wrl.h_d3d11_alias.h"

class Render;

class Texture {
private:
	comptr<ID3D11Texture2D> m_texture;

public:
	ID3D11Texture2D* get() { return m_texture.Get(); }
};