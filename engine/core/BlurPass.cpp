#include "BlurPass.h"

#include "Render.h"
#include "CameraComponent.h"
#include "MaterialAlias.h"
#include "Mesh.h"
#include "MeshComponent.h"
#include "DirectionLight.h"

#include "RenderTarget.h"
#include "ShaderResource.h"
#include "DepthStencil.h"


void BlurPass::Init(Game* game) {
	RenderPass::Init(game);

	m_screenQuad.Init(m_game->render(), m_game->shaderAsset()->GetShader(Assets::ShaderBlur));

	auto width = m_game->window()->GetWidth();
	auto height = m_game->window()->GetHeight();

	Resize(width, height);
	SetRenderTargets({ &target0 });
}

void BlurPass::Resize(float width, float height) {
	target0Tex = Texture::Create(m_game->render(), width, height, DXGI_FORMAT_R8G8B8A8_UNORM);
	tmpTarget1Tex = Texture::Create(m_game->render(), width, height, DXGI_FORMAT_R8G8B8A8_UNORM);
	tmpTarget2Tex = Texture::Create(m_game->render(), width, height, DXGI_FORMAT_R8G8B8A8_UNORM);

	target0 = RenderTarget::Create(&target0Tex);
	tmpTarget1 = RenderTarget::Create(&tmpTarget1Tex);
	tmpTarget2 = RenderTarget::Create(&tmpTarget2Tex);

	target0Res = ShaderResource::Create(&target0Tex);
	tmpTarget1Res = ShaderResource::Create(&tmpTarget1Tex);
	tmpTarget2Res = ShaderResource::Create(&tmpTarget2Tex);
}

void BlurPass::Draw() {
	if (!m_game->ui()->HasActor()) {
		BeginDraw();
		EndDraw();
	}

	auto source = m_PSResources[0];
	auto destination = &target0;

	ShaderResource* tmpRes[2] = { &tmpTarget1Res, &tmpTarget2Res };
	RenderTarget* tmpTar[2] = { &tmpTarget1, &tmpTarget2 };

	if (passesCount > 1) {
		m_Draw(source, tmpTar[0]);

		for (int i = 0; i < passesCount - 2; i++) {
			m_Draw(tmpRes[0], tmpTar[1]);

			std::swap(tmpRes[0], tmpRes[1]);
			std::swap(tmpTar[0], tmpTar[1]);
		}
		m_Draw(tmpRes[0], destination);
	}
	else {
		m_Draw(source, destination);
	}
	SetPSShaderResources({ source });
}

void BlurPass::m_Draw(ShaderResource* resource, RenderTarget* target) {
	SetPSShaderResources({ resource });
	SetRenderTargets({ target });

	BeginDraw();
	m_screenQuad.Draw();
	EndDraw();
}