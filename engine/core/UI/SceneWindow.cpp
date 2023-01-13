#include "SceneWindow.h"

#include "UI_Editor.h"
#include "../Game.h"
#include "../Scene.h"
#include "../Render.h"
#include "../HotKeys.h"
#include "UserInterface.h"
#include "../CameraComponent.h"
#include "../ImageAsset.h"
#include "../Assets.h"


const std::string& SceneWindow::name() { 
	if (scene() != nullptr)
		return  scene()->name();
	return sceneId; 
}

void SceneWindow::Draw() {
	if (!visible)
		return;
	
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

	auto flags = 0;
	if (scene() != nullptr && game()->assets()->IsDirty(scene()->assetIdHash()))
		flags = ImGuiWindowFlags_UnsavedDocument;

	if (ImGui::Begin(windowId().c_str(), &visible, flags)) {
		m_game->ui()->selectedScene(scene());

		m_UpdateViewportInfo();

		if (scene() != nullptr) {
			m_game->mainCamera(scene()->mainCamera());
			m_game->PushScene(scene());

			m_DrawRender();
			AfterDrawScene();

			m_game->PopScene();
		}
	}
	ImGui::End();
	ImGui::PopStyleVar();
}

void SceneWindow::m_DrawRender() {
	auto* renderer = &scene()->renderer;

	renderer->ResizeViewport(m_viewportSize);
	ImGui::Image(renderer->screenSRV(), (ImVec2&)m_viewportSize);
}

void SceneWindow::Init(Game* game) {
	m_game = game;
	OnInit();
}

void SceneWindow::m_UpdateViewportInfo() {
	m_viewportSize = (Vector2&)ImGui::GetWindowSize() - Vector2(0, 19.0f);

	m_viewportPosition = (Vector2&)ImGui::GetWindowPos();

	auto mousePos = (Vector2&)ImGui::GetMousePos();
	mousePos -= (Vector2&)ImGui::GetWindowPos() + Vector2(0, 19.0f);
	mousePos = mousePos / m_viewportSize;

	m_mouseViewportPosition = mousePos;
	game()->ui()->isSceneHovered = ImGui::IsWindowHovered();
}

void SceneWindow::focus() {
	ImGui::SetWindowFocus(windowId().c_str());
}
