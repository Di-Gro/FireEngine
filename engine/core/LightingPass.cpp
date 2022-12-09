#include "LightingPass.h"

#include "Render.h"
#include "CameraComponent.h"
#include "MaterialAlias.h"
#include "Mesh.h"
#include "MeshComponent.h"
#include "DirectionLight.h"

#include "RenderTarget.h"
#include "ShaderResource.h"
#include "DepthStencil.h"


#pragma pack(push, 4)
static struct DirectionLightCBuffer {
	Matrix uvMatrix;
	Vector3 direction;
	float intensity;
	Vector3 color;
	float _1[1];
};
#pragma pack(pop)


void LightingPass::Init(Game* game) {
	RenderPass::Init(game);

	//blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	//blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;

	//UpdateBlendState();

	D3D11_BUFFER_DESC dirLightCBufferDesc = {};
	dirLightCBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	dirLightCBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	dirLightCBufferDesc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
	dirLightCBufferDesc.MiscFlags = 0;
	dirLightCBufferDesc.StructureByteStride = 0;
	dirLightCBufferDesc.ByteWidth = sizeof(DirectionLightCBuffer);

	m_render->device()->CreateBuffer(&dirLightCBufferDesc, nullptr, m_directionLightCBuffer.GetAddressOf());

	m_screenQuad.Init(m_render, m_game->shaderAsset()->GetShader("../../data/engine/shaders/screen.hlsl"));
}

void LightingPass::Draw() {
	auto* context = m_render->context();

	BeginDraw();
	m_SetLightConstBuffer();
	context->RSSetState(m_render->rastCullBack.Get());

	m_screenQuad.Draw();

	EndDraw();
}

void LightingPass::m_SetLightConstBuffer() {
	auto* context = m_render->context();
	auto* dirLight = m_game->lighting()->directionLight();

	context->PSSetConstantBuffers(Buf_LightingPass_Light_PS, 1, m_directionLightCBuffer.GetAddressOf());

	D3D11_MAPPED_SUBRESOURCE res3 = {};
	context->Map(m_directionLightCBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &res3);

	auto* cbuf2 = (DirectionLightCBuffer*)res3.pData;
	cbuf2->uvMatrix = dirLight->uvMatrix();
	cbuf2->direction = dirLight->forward();
	cbuf2->color = dirLight->color;
	cbuf2->intensity = dirLight->intensity;

	context->Unmap(m_directionLightCBuffer.Get(), 0);
}