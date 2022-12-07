#include "OpaquePass.h"

#include "OldPass.h"
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


void OpaquePass::Init(Game* game) {
	RenderPass::Init(game);

	m_game = game;

	D3D11_BUFFER_DESC dirLightCBufferDesc = {};
	dirLightCBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	dirLightCBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	dirLightCBufferDesc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
	dirLightCBufferDesc.MiscFlags = 0;
	dirLightCBufferDesc.StructureByteStride = 0;
	dirLightCBufferDesc.ByteWidth = sizeof(DirectionLightCBuffer);

	m_render->device()->CreateBuffer(&dirLightCBufferDesc, nullptr, m_directionLightCBuffer.GetAddressOf());

}

void OpaquePass::Draw() {

	BeginDraw();
	m_SetLightConstBuffer();

	for (auto& pair : f_sortedMaterials) {
		auto *matShapes = &pair.second;
		auto* material = matShapes->material;
		auto* shapes = matShapes->shapes;

		bool once = true;
		
		for (auto shapeRef : *shapes) {
			auto *meshComponent = shapeRef.first;
			auto index = shapeRef.second;

			if (!meshComponent->IsDestroyed() && meshComponent->IsDrawable()) {
				if (once) {
					once = false;
					PrepareMaterial(material);
				}
				meshComponent->OnDrawShape(index);
			}
		}
	}

	EndDraw();
}

void OpaquePass::m_SetLightConstBuffer() {
	auto* context = m_render->context();
	auto* dirLight = m_game->lighting()->directionLight();

	context->PSSetConstantBuffers(Buf_OpaquePass_Light_PS, 1, m_directionLightCBuffer.GetAddressOf());

	D3D11_MAPPED_SUBRESOURCE res3 = {};
	context->Map(m_directionLightCBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &res3);

	auto* cbuf2 = (DirectionLightCBuffer*)res3.pData;
	cbuf2->uvMatrix = dirLight->uvMatrix();
	cbuf2->direction = dirLight->forward();
	cbuf2->color = dirLight->color;
	cbuf2->intensity = dirLight->intensity;

	context->Unmap(m_directionLightCBuffer.Get(), 0);
}