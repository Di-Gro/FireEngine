#pragma once

#include "wrl.h_d3d11_alias.h"


class ShaderResource {
private:
	comptr<ID3D11ShaderResourceView> m_renderSRV;

public:
	ID3D11ShaderResourceView* get() { return m_renderSRV.Get(); }
};