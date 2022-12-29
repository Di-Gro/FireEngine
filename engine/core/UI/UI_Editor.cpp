#include "UI_Editor.h"
#include "../Game.h"
#include "../Render.h"
#include "../HotKeys.h"
#include "UserInterface.h"
#include "../CameraComponent.h"
#include "../ImageAsset.h"
#include "../Assets.h"

bool UI_Editor::ButtonCenteredOnLine(const char* label, float alignment)
{
	ImGuiStyle& style = ImGui::GetStyle();

	float size = ImGui::CalcTextSize(label).x + style.FramePadding.x * 2.0f;
	float avail = ImGui::GetContentRegionAvail().x;

	float off = (avail - size) * alignment;
	if (off > 0.0f)
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);

	return ImGui::Button(label);
}

void UI_Editor::Draw_UI_Editor()
{
	bool hasClickInViewport = false;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	if (ImGui::Begin("Scene"))
	{
		m_UpdateViewportRect();
		m_UpdateViewportMousePos();
		m_DrawRender();

		m_HandleEditorInput();
		
		Draw_Tools();
		m_DrawGuizmo();
	}
	ImGui::End();
	ImGui::PopStyleVar();

	m_HandleSelection();
}

void UI_Editor::m_DrawRender() {
	_game->render()->ResizeViewport(m_viewportSize);
	ImGui::Image(_game->render()->screenSRV(), (ImVec2&)m_viewportSize);
}

void UI_Editor::Draw_Tools()
{
	if (_game->inFocus)
		return;

	ImGuiWindowFlags window_flags = 0
		| ImGuiWindowFlags_NoDocking
		| ImGuiWindowFlags_NoTitleBar
		| ImGuiWindowFlags_NoResize
		| ImGuiWindowFlags_NoMove
		| ImGuiWindowFlags_NoScrollbar
		| ImGuiWindowFlags_NoSavedSettings
		| ImGuiWindowFlags_NoNavFocus;

	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
	ImGuiWindowClass tools;
	tools.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_AutoHideTabBar;
	ImGui::SetNextWindowPos({ ImGui::GetWindowPos().x + 5.0f, ImGui::GetWindowPos().y + 25.0f });
	ImGui::SetNextWindowSize({ 46, 142 });
	ImGui::SetNextWindowClass(&tools);
	if (ImGui::Begin("##tools", NULL, window_flags))
	{
		auto currentMode = m_CurrentGizmoMode == ImGuizmo::LOCAL ? "Local" : "World";

		/// TODO: Сделать с правой стороны
		ImGui::PushItemWidth(30.0f);
		if (ImGui::Button(currentMode))
			m_ToggleGizmoMode();

		ImGui::PopItemWidth();

		m_DrawTransfotmButton(&m_moveRes, ImGuizmo::TRANSLATE);
		m_DrawTransfotmButton(&m_rotateRes, ImGuizmo::ROTATE);
		m_DrawTransfotmButton(&m_scaleRes, ImGuizmo::SCALE);
	}
	ImGui::End();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
}

void UI_Editor::Init(Game* game)
{
	_game = game;
	viewport = ImGui::GetMainViewport();

	m_InitIcons();
}

void UI_Editor::m_InitIcons() {
	_game->imageAsset()->InitImage(&m_icMove, "../../data/engine/icons/ic_move.png");
	_game->imageAsset()->InitImage(&m_icRotate, "../../data/engine/icons/ic_rotate.png");
	_game->imageAsset()->InitImage(&m_icScale, "../../data/engine/icons/ic_scale.png");

	m_moveTex = Texture::CreateFromImage(_game->render(), &m_icMove);
	m_rotateTex = Texture::CreateFromImage(_game->render(), &m_icRotate);
	m_scaleTex = Texture::CreateFromImage(_game->render(), &m_icScale);

	m_moveRes = ShaderResource::Create(&m_moveTex);
	m_rotateRes = ShaderResource::Create(&m_rotateTex);
	m_scaleRes = ShaderResource::Create(&m_scaleTex);
}

void UI_Editor::m_DrawGuizmo() {
	auto actor = _game->ui()->GetActor();
	if (actor == nullptr)
		return;

	ImGuizmo::SetDrawlist();

	auto m = actor->GetWorldMatrix();
	auto inv = Matrix::Identity;
	if (actor->parent() != nullptr)
		inv = actor->parent()->GetWorldMatrix().Invert();

	auto matrix = (ImGuizmo::matrix_t&)m;
	auto camera = _game->mainCamera();
	auto mView = (ImGuizmo::matrix_t&)camera->viewMatrix();
	auto mProjection = (ImGuizmo::matrix_t&)camera->projMatrix();

	ImGuiIO& io = ImGui::GetIO();
	auto vsize = _game->render()->viewportSize();
	auto vpos = _game->ui()->viewportPosition(); 

	ImGuizmo::SetRect(vpos.x, vpos.y, vsize.x, vsize.y);
	ImGuizmo::Manipulate(mView.m16, mProjection.m16, m_CurrentGizmoOperation, m_CurrentGizmoMode, matrix.m16, NULL, NULL);

	m = (Matrix)matrix * inv;
	actor->SetLocalMatrix(m);
}

void UI_Editor::m_DrawTransfotmButton(ShaderResource* icon, ImGuizmo::OPERATION target) {
	auto isSelected = m_CurrentGizmoOperation == target;
	if (isSelected)
		ImGui::PushStyleColor(ImGuiCol_Button, m_selectedColor); 
	
	if (ImGui::ImageButton(icon->get(), m_iconSize))
		m_CurrentGizmoOperation = target;

	if (isSelected)
		ImGui::PopStyleColor();
}

void UI_Editor::m_ToggleGizmoMode() {
	m_CurrentGizmoMode = m_CurrentGizmoMode == ImGuizmo::LOCAL ? ImGuizmo::WORLD : ImGuizmo::LOCAL;
}

void UI_Editor::m_HandleEditorInput() {
	if (_game->hotkeys()->GetButtonDown(Keys::Q))
		m_ToggleGizmoMode();

	if (_game->hotkeys()->GetButtonDown(Keys::E))
		m_CurrentGizmoOperation = ImGuizmo::TRANSLATE;

	if (_game->hotkeys()->GetButtonDown(Keys::R))
		m_CurrentGizmoOperation = ImGuizmo::ROTATE;

	if (_game->hotkeys()->GetButtonDown(Keys::T))
		m_CurrentGizmoOperation = ImGuizmo::SCALE;

	auto hasClick = ImGui::IsMouseClicked(ImGuiMouseButton_::ImGuiMouseButton_Left);
	m_hasClickInViewport = hasClick && ImGui::IsWindowHovered();
}

void UI_Editor::m_HandleSelection() {
	if (m_hasClickInViewport && !ImGuizmo::IsOver() && !_game->inFocus) {
		auto vpos = _game->ui()->mouseViewportPosition();
		auto actorRef = _game->render()->GetActorIdInViewport(vpos);
		if (actorRef != 0) {
			auto actor = CppRefs::ThrowPointer<Actor>(RefCpp(actorRef));
			_game->ui()->SelectedActor(actor);
		}
		else {
			_game->ui()->SelectedActor(nullptr);
		}
	}
}

void UI_Editor::m_UpdateViewportRect() {
	m_viewportSize = (Vector2&)ImGui::GetWindowSize() - Vector2(0, 19.0f);

	_game->ui()->f_viewportPosition = (Vector2&)ImGui::GetWindowPos();
}

void UI_Editor::m_UpdateViewportMousePos() {
	auto mousePos = (Vector2&)ImGui::GetMousePos();
	mousePos -= (Vector2&)ImGui::GetWindowPos() + Vector2(0, 19.0f);
	mousePos = mousePos / m_viewportSize;

	_game->ui()->f_mouseViewportPosition = mousePos;
}