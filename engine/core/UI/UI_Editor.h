#pragma once
#include "../imgui/imgui.h"
#include "../imguizmo/ImGuizmo.h"
#include "../SimpleMath.h"
#include "../Texture.h"
#include "../ShaderResource.h"
#include "../ImageAsset.h"

class Game;

class UI_Editor
{
public:
	void Draw_UI_Editor();
	void Draw_Tools();
	void Init(Game* game);
	bool ButtonCenteredOnLine(const char* label, float alignment = 0.5f);

private:
	Game* _game;
	const ImGuiViewport* viewport;

	ImGuizmo::OPERATION m_CurrentGizmoOperation = ImGuizmo::TRANSLATE;
	ImGuizmo::MODE m_CurrentGizmoMode = ImGuizmo::LOCAL;

	bool m_hasClickInViewport = false;
	Vector2 m_viewportSize;

	Image m_icMove;
	Image m_icRotate;
	Image m_icScale;

	Texture m_moveTex;
	Texture m_rotateTex;
	Texture m_scaleTex;

	ShaderResource m_moveRes;
	ShaderResource m_rotateRes;
	ShaderResource m_scaleRes;

	ImVec4 m_selectedColor = ImVec4(71 / 255.0f, 114 / 255.0f, 179 / 255.0f, 1.0f);
	ImVec2 m_iconSize = ImVec2(28, 28);

private:
	void m_InitIcons();

	void m_DrawGuizmo();
	void m_DrawRender();
	void m_DrawTransfotmButton(ShaderResource* icon, ImGuizmo::OPERATION target);

	void m_ToggleGizmoMode();
	void m_HandleEditorInput();
	void m_HandleSelection();
	void m_UpdateViewportRect();
	void m_UpdateViewportMousePos();
};