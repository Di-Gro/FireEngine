#pragma once
#include "SceneWindow.h"

#include "../imguizmo/ImGuizmo.h"
#include "../SimpleMath.h"

#include "../Texture.h"
#include "../ShaderResource.h"
#include "../ImageAsset.h"

class Game;

class SceneEditorWindow : public SceneWindow {

private:

	ImGuizmo::OPERATION m_CurrentGizmoOperation = ImGuizmo::TRANSLATE;
	ImGuizmo::MODE m_CurrentGizmoMode = ImGuizmo::LOCAL;

	bool m_hasClickInViewport = false;

	ImVec4 m_selectedColor = ImVec4(71 / 255.0f, 114 / 255.0f, 179 / 255.0f, 1.0f);
	ImVec2 m_iconSize = ImVec2(28, 28);

public:
	SceneEditorWindow(const std::string& _sceneId) : SceneWindow(_sceneId) { }

protected:
	void OnInit() override;
	void AfterDrawScene() override;

private:
	void m_DrawTools();
	void m_DrawGuizmo();
	void m_DrawTransfotmButton(ShaderResource* icon, ImGuizmo::OPERATION target);

	void m_ToggleGizmoMode();
	void m_HandleEditorInput();
	void m_HandleSelection();

};

