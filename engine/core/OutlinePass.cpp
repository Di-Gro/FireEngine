#include "OutlinePass.h"

#include "Game.h"
#include "MeshComponent.h"

void OutlinePass::Init(Game* game) {
	RenderPass::Init(game);

	blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;

	UpdateBlendState();

	m_screenQuad.Init(m_game->render(), m_game->shaderAsset()->GetShader(Assets::ShaderEditorOutline));

	auto width = m_game->window()->GetWidth();
	auto height = m_game->window()->GetHeight();

	Resize(width, height);
}

void OutlinePass::Draw() {
	BeginDraw();

	if (m_game->ui()->HasActor())
		m_screenQuad.Draw();

	EndDraw();
}

