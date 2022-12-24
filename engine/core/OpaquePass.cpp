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


void OpaquePass::Init(Game* game) {
	RenderPass::Init(game);

	auto blendState = blendStateDesc.RenderTarget[0];

	blendStateDesc = D3D11_BLEND_DESC{ false, true };
	blendStateDesc.RenderTarget[0] = blendState;
	blendStateDesc.RenderTarget[1] = blendState;
	blendStateDesc.RenderTarget[2] = blendState;
	blendStateDesc.RenderTarget[3] = blendState;
	blendStateDesc.RenderTarget[4] = blendState;
	blendStateDesc.RenderTarget[5] = blendState;
	blendStateDesc.RenderTarget[5].BlendEnable = false;

	UpdateBlendState();

	auto width = m_game->window()->GetWidth();
	auto height = m_game->window()->GetHeight();

	Resize(width, height);
}

void OpaquePass::Resize(float width, float height) {
	target0Tex = Texture::Create(m_game->render(), width, height, DXGI_FORMAT_R8G8B8A8_UNORM);
	target1Tex = Texture::Create(m_game->render(), width, height, DXGI_FORMAT_R32G32B32A32_FLOAT);
	target2Tex = Texture::Create(m_game->render(), width, height, DXGI_FORMAT_R8G8B8A8_UNORM);
	target3Tex = Texture::Create(m_game->render(), width, height, DXGI_FORMAT_R32G32B32A32_FLOAT);
	target4Tex = Texture::Create(m_game->render(), width, height, DXGI_FORMAT_R32G32B32A32_FLOAT);
	target5Tex = Texture::Create(m_game->render(), width, height, DXGI_FORMAT_R32G32_UINT);

	target0 = RenderTarget::Create(&target0Tex);
	target1 = RenderTarget::Create(&target1Tex);
	target2 = RenderTarget::Create(&target2Tex);
	target3 = RenderTarget::Create(&target3Tex);
	target4 = RenderTarget::Create(&target4Tex);
	target5 = RenderTarget::Create(&target5Tex);

	target0Res = ShaderResource::Create(&target0Tex);
	target1Res = ShaderResource::Create(&target1Tex);
	target2Res = ShaderResource::Create(&target2Tex);
	target3Res = ShaderResource::Create(&target3Tex);
	target4Res = ShaderResource::Create(&target4Tex);
	target5Res = ShaderResource::Create(&target5Tex);
}

void OpaquePass::Draw() {

	BeginDraw();

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
				SetActorConstBuffer(meshComponent->actor());
				meshComponent->OnDrawShape(index);
			}
		}
	}

	EndDraw();
}
