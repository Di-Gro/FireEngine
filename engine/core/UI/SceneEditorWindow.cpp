#include "SceneEditorWindow.h"

#include "../Game.h"
#include "../Scene.h"
#include "../Render.h"
#include "../HotKeys.h"
#include "UserInterface.h"
#include "../CameraComponent.h"
#include "../ImageAsset.h"
#include "../Assets.h"


void SceneEditorWindow::OnInit() {

}


void SceneEditorWindow::AfterDrawScene() {
	if (!scene()->isEditor())
		return;

	m_HandleEditorInput();

	m_DrawTools();
	m_DrawGuizmo();
	m_HandleSelection();
}

void SceneEditorWindow::m_DrawTools()
{
	if (game()->inFocus)
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

		m_DrawTransfotmButton(&game()->ui()->icMove, ImGuizmo::TRANSLATE);
		m_DrawTransfotmButton(&game()->ui()->icRotate, ImGuizmo::ROTATE);
		m_DrawTransfotmButton(&game()->ui()->icScale, ImGuizmo::SCALE);
	}
	ImGui::End();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
}

void SceneEditorWindow::m_DrawGuizmo() {
	auto actor = game()->ui()->GetActor();
	auto camera = scene()->mainCamera();

	if (actor == nullptr || camera == nullptr)
		return;

	ImGuizmo::SetDrawlist();

	auto m = actor->GetWorldMatrix();
	auto inv = Matrix::Identity;
	if (actor->parent() != nullptr)
		inv = actor->parent()->GetWorldMatrix().Invert();

	auto matrix = (ImGuizmo::matrix_t&)m;
	auto mView = (ImGuizmo::matrix_t&)camera->viewMatrix();
	auto mProjection = (ImGuizmo::matrix_t&)camera->projMatrix();

	ImGuiIO& io = ImGui::GetIO();
	auto vsize = viewportSize();
	auto vpos = viewportPosition();

	auto wpos = actor->localPosition();
	auto scale = actor->localScale();
	Vector3 bound[] = { -scale/2, scale/2 };

	ImGuizmo::SetRect(vpos.x, vpos.y, vsize.x, vsize.y);
	ImGuizmo::Manipulate(mView.m16, mProjection.m16,
		m_CurrentGizmoOperation,
		m_CurrentGizmoMode,
		matrix.m16,
		NULL,
		NULL,
		m_CurrentGizmoOperation == ImGuizmo::BOUNDS ? &bound[0].x : NULL,
		NULL
	);

	m = (Matrix)matrix * inv;
	actor->SetLocalMatrix(m);
}

void SceneEditorWindow::m_DrawTransfotmButton(ShaderResource* icon, ImGuizmo::OPERATION target) {
	auto isSelected = m_CurrentGizmoOperation == target;
	if (isSelected)
		ImGui::PushStyleColor(ImGuiCol_Button, m_selectedColor);

	if (ImGui::ImageButton(icon->get(), m_iconSize))
		m_CurrentGizmoOperation = target;

	if (isSelected)
		ImGui::PopStyleColor();
}

void SceneEditorWindow::m_ToggleGizmoMode() {
	m_CurrentGizmoMode = m_CurrentGizmoMode == ImGuizmo::LOCAL ? ImGuizmo::WORLD : ImGuizmo::LOCAL;
}

void SceneEditorWindow::m_HandleEditorInput() {
	if (game()->hotkeys()->GetButtonDown(Keys::Q))
		m_ToggleGizmoMode();

	if (game()->hotkeys()->GetButtonDown(Keys::E))
		m_CurrentGizmoOperation = ImGuizmo::TRANSLATE;

	if (game()->hotkeys()->GetButtonDown(Keys::R))
		m_CurrentGizmoOperation = ImGuizmo::ROTATE;

	if (game()->hotkeys()->GetButtonDown(Keys::T))
		m_CurrentGizmoOperation = ImGuizmo::SCALE;

	if (game()->hotkeys()->GetButtonDown(Keys::L))
		m_CurrentGizmoOperation = ImGuizmo::BOUNDS;

	auto hasClick = ImGui::IsMouseClicked(ImGuiMouseButton_::ImGuiMouseButton_Left);
	m_hasClickInViewport = hasClick && ImGui::IsWindowHovered();
}

void SceneEditorWindow::m_HandleSelection() {
	if (m_hasClickInViewport && !ImGuizmo::IsOver() && !game()->inFocus) {
		auto vpos = mouseViewportPosition(); //game()->ui()->mouseViewportPosition();
		auto actorRef = scene()->renderer.GetActorIdInViewport(vpos);
		if (actorRef != 0) {
			auto actor = CppRefs::ThrowPointer<Actor>(RefCpp(actorRef));
			game()->ui()->SelectedActor(actor);
		}
		else {
			game()->ui()->SelectedActor(nullptr);
		}
	}
}
