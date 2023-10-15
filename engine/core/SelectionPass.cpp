#include "SelectionPass.h"

#include "Game.h"
#include "Window.h"
#include "Assets.h"
#include "UI/UserInterface.h"
#include "Actor.h"
#include "MeshComponent.h"
#include "MaterialAsset.h"

void SelectionPass::Init(Game* game) {
	RenderPass::Init(game);

	m_highlightMaterial = MaterialAsset::CreateDynamic(m_game, "Editor Highlight", Assets::ShaderEditorHihglight);
	m_highlightMaterial->resource.cullMode = CullMode::None;

	auto width = m_game->window()->GetWidth();
	auto height = m_game->window()->GetHeight();

	Resize(width, height);

	SetRenderTargets({ &target0 });
}

void SelectionPass::Resize(float width, float height) {
	target0Tex = TextureResource::Create(m_game->render(), width, height, DXGI_FORMAT_R8G8B8A8_UNORM);
	target0 = RenderTarget::Create(&target0Tex);
	target0Res = ShaderResource::Create(&target0Tex);
}

void SelectionPass::Draw() {
	BeginDraw();

	if (m_game->ui()->HasActor()) {
		SetEditorConstBuffer();
		PrepareMaterial(&m_highlightMaterial->resource);

		m_DrawActor(m_game->ui()->GetActor());
	}

	EndDraw();
}

void SelectionPass::m_DrawActor(Actor* actor) {
	if (actor->IsDestroyed())
		return;

	auto components = actor->GetComponentList();
	for (auto component : *components) {
		auto meshComppnent = dynamic_cast<MeshComponent*>(component);
		if (meshComppnent != nullptr)
			meshComppnent->OnDraw();
	}
	auto childrenCount = actor->GetChildrenCount();
	for (int i = 0; i < childrenCount; i++)
		m_DrawActor(actor->GetChild(i));

}
