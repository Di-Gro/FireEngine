#include "ScreenQuad.h"

#include "Render.h"
#include "RenderPass.h"
#include "CameraComponent.h"

void ScreenQuad::Init(Render* render, const Shader* shader) {
	m_render = render;
	this->shader = shader;

	D3D11_SAMPLER_DESC sampleDesc = {};
	sampleDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampleDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampleDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampleDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampleDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	sampleDesc.BorderColor[0] = 1.0f;
	sampleDesc.BorderColor[1] = 0.0f;
	sampleDesc.BorderColor[2] = 0.0f;
	sampleDesc.BorderColor[3] = 1.0f;
	sampleDesc.MaxLOD = INT_MAX;

	m_render->device()->CreateSamplerState(&sampleDesc, m_sampler.GetAddressOf());

	CD3D11_RASTERIZER_DESC rastDesc = {};
	rastDesc.CullMode = D3D11_CULL_FRONT;
	rastDesc.FillMode = D3D11_FILL_SOLID;

	m_render->device()->CreateRasterizerState(&rastDesc, rastState.GetAddressOf());
}

void ScreenQuad::Release() {
	if (m_sampler.Get() != nullptr)
		m_sampler.ReleaseAndGetAddressOf();

	if (rastState.Get() != nullptr)
		m_sampler.ReleaseAndGetAddressOf();
}

void ScreenQuad::Draw() const {
	auto* context = m_render->context();
	auto* camera = m_render->renderer()->camera();

	/// Material: SetResources
	ID3D11ShaderResourceView* resources[] = { deffuse != nullptr ? deffuse->get() : nullptr };
	context->PSSetShaderResources(PASS_R_MATERIAL_PS, 1, resources);
	context->PSSetSamplers(PASS_R_MATERIAL_PS, 1, m_sampler.GetAddressOf());

	/// Material: SetShader
	context->IASetInputLayout(nullptr);
	context->VSSetShader(shader->vertex.Get(), nullptr, 0);
	context->PSSetShader(shader->pixel.Get(), nullptr, 0);

	/// Material: SetMaterialConstBuffer 
	/// Material: SetRasterizerState
	/// Mesh: SetTopology
	context->IASetPrimitiveTopology(topology);

	/// Mesh: SetInput
	context->IASetIndexBuffer(nullptr, DXGI_FORMAT_R32_UINT, 0);

	/// Mesh: SetConstBuffers
	/// Mesh: Draw
	context->Draw(4, 0);
}

void ScreenQuad::Draw2() const {
	auto* context = m_render->context();
	auto* camera = m_render->renderer()->camera();

	/// Mesh: SetTopology
	context->IASetPrimitiveTopology(topology);

	/// Mesh: SetInput
	context->IASetIndexBuffer(nullptr, DXGI_FORMAT_R32_UINT, 0);

	/// Mesh: SetConstBuffers
	/// Mesh: Draw
	context->Draw(4, 0);
}