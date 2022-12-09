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

	auto width = m_game->window()->GetWidth();
	auto height = m_game->window()->GetHeight();

	target0Tex = Texture::Create(m_game->render(), width, height);
	target1Tex = Texture::Create(m_game->render(), width, height);
	target2Tex = Texture::Create(m_game->render(), width, height);
	target3Tex = Texture::Create(m_game->render(), width, height);
	target4Tex = Texture::Create(m_game->render(), width, height);

	target0 = RenderTarget::Create(&target0Tex);
	target1 = RenderTarget::Create(&target1Tex);
	target2 = RenderTarget::Create(&target2Tex);
	target3 = RenderTarget::Create(&target3Tex);
	target4 = RenderTarget::Create(&target4Tex);

	target0Res = ShaderResource::Create(&target0Tex);
	target1Res = ShaderResource::Create(&target1Tex);
	target2Res = ShaderResource::Create(&target2Tex);
	target3Res = ShaderResource::Create(&target3Tex);
	target4Res = ShaderResource::Create(&target4Tex);

	//SetRenderTargets({ &target0, &target1, &target2, &target3, &target4 });
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
				meshComponent->OnDrawShape(index);
			}
		}
	}

	EndDraw();
}
