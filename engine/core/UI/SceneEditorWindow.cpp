#include "SceneEditorWindow.h"

#include "UserInterface.h"

#include "../Game.h"
#include "../Window.h"
#include "../Scene.h"
#include "../Audio.h"
#include "../Render.h"
#include "../HotKeys.h"
#include "../CameraComponent.h"
#include "../EditorCamera.h"
#include "../ImageAsset.h"
#include "../Assets.h"
#include "../ShaderResource.h"


void SceneEditorWindow::OnInit() {

}

void SceneEditorWindow::AfterDrawScene() {
	if (scene()->isEditor() || !game()->inFocus) {
		m_HandleToolsInput();
		//m_DrawTools();
		m_DrawGuizmo();
		m_HandleSelection();
	}
	if (scene()->isEditor()) {
		if (ImGui::IsWindowFocused() && game()->hotkeys()->GetButtonDownEd(Keys::S, Keys::Ctrl))
			game()->assets()->Save(scene()->assetIdHash());
	}
	if (!scene()->isEditor() && game()->inFocus) {
		auto vpos = viewportPosition();
		auto vsize = viewportSize();

		game()->window()->ClipCursor(vpos.x + 4, vpos.y + m_barHeight + 20, vsize.x - 8, vsize.y - 5);
	}
	else if (game()->window()->IsCursorClipped()) {
		game()->window()->UnclipCursor();
	}
}

void SceneEditorWindow::OnDrawBar() {
	ImVec2 iconSize = { 20, 20 };

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 5.0f, 0.0f });
	ImGui::Dummy({ 0, 3 });
	ImGui::Indent(10);

	m_DrawAudioVolume(iconSize);

	ImGui::SameLine();
	m_DrawSeparator();

	if (game()->inFocus) {
		ImGui::SameLine();
		ImGui::TextDisabled("Press '~' tp exit focus mode");

		//auto textSize = ImGui::CalcTextSize("Pr");
		//ImGui::Dummy({ 0, iconSize.y - textSize.y });
	}
	else {
		auto currentMode = m_CurrentGizmoMode == ImGuizmo::LOCAL ? "Local" : "World";

		ImGui::SameLine();
		if (ImGui::Button(currentMode, { 80, 20 }))
			m_ToggleGizmoMode();

		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, { 0.0f, 0.0f });
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0.0f, 0.0f });
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

		ImGui::SameLine();
		m_DrawTransfotmButton(&game()->ui()->icMove, iconSize, ImGuizmo::TRANSLATE);
		ImGui::SameLine();
		m_DrawTransfotmButton(&game()->ui()->icRotate, iconSize, ImGuizmo::ROTATE);
		ImGui::SameLine();
		m_DrawTransfotmButton(&game()->ui()->icScale, iconSize, ImGuizmo::SCALE);

		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(1);

		ImGui::SameLine();
		m_DrawSeparator();

		if (scene()->editorCamera != nullptr) {
			ImGui::SameLine();
			ImGui::SetNextItemWidth(80);
			ImGui::DragFloat("##EditorCameraSpeed", &scene()->editorCamera->speed, 1, 0, 10000);
		}
	}
	ImGui::Unindent(10);
	ImGui::Dummy({ 0, 3 });
	ImGui::PopStyleVar(1);
}


void SceneEditorWindow::m_DrawSeparator() {
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0.0f, 0.0f });
	ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, { 0.0f, 0.0f });

	ImGui::Dummy({ 3, 0 });
	ImGui::SameLine();
	ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
	ImGui::SameLine();
	ImGui::Dummy({ 3, 0 });

	ImGui::PopStyleVar(2);
}

//void SceneEditorWindow::m_DrawTools()
//{
//	if (game()->inFocus)
//		return;
//
//	ImGuiWindowFlags window_flags = 0
//		| ImGuiWindowFlags_NoDocking
//		| ImGuiWindowFlags_NoTitleBar
//		| ImGuiWindowFlags_NoResize
//		| ImGuiWindowFlags_NoMove
//		| ImGuiWindowFlags_NoScrollbar
//		| ImGuiWindowFlags_NoSavedSettings
//		| ImGuiWindowFlags_NoNavFocus;
//
//	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
//	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
//	ImGuiWindowClass tools;
//	tools.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_AutoHideTabBar;
//
//	float beginHeight = ImGui::GetWindowPos().y + m_beginViewHeight + 5;
//
//	ImGui::SetNextWindowPos({ ImGui::GetWindowPos().x + 5.0f, beginHeight });
//	ImGui::SetNextWindowSize({ 46, 142 });
//	ImGui::SetNextWindowClass(&tools);
//	if (ImGui::Begin("##tools", NULL, window_flags))
//	{
//		auto currentMode = m_CurrentGizmoMode == ImGuizmo::LOCAL ? "Local" : "World";
//
//		/// TODO: Сделать с правой стороны
//		ImGui::PushItemWidth(30.0f);
//		if (ImGui::Button(currentMode))
//			m_ToggleGizmoMode();
//
//		ImGui::PopItemWidth();
//
//		m_DrawTransfotmButton(&game()->ui()->icMove, m_iconSize, ImGuizmo::TRANSLATE);
//		m_DrawTransfotmButton(&game()->ui()->icRotate, m_iconSize, ImGuizmo::ROTATE);
//		m_DrawTransfotmButton(&game()->ui()->icScale, m_iconSize, ImGuizmo::SCALE);
//	}
//	ImGui::End();
//	ImGui::PopStyleColor();
//	ImGui::PopStyleColor();
//
//	//if (scene()->editorCamera != nullptr) {
//	//	auto pos = (Vector2&)ImGui::GetWindowPos();
//	//	auto size = (Vector2&)ImGui::GetWindowSize();
//
//	//	auto lastCursor = (Vector2&)ImGui::GetCursorPos();
//	//	auto startPos = lastCursor + Vector2(50, -size.y + m_barHeight);
//
//	//	ImGui::SetCursorPos((ImVec2&)startPos);
//
//	//	ImGui::SetNextItemWidth(80);
//	//	ImGui::DragFloat("##EditorCameraSpeed", &scene()->editorCamera->speed, 1, 0, 10000);
//
//	//	ImGui::SetCursorPos((ImVec2&)lastCursor);
//	//}
//}


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
	bool changed = ImGuizmo::Manipulate(mView.m16, mProjection.m16,
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

	if (changed)
		game()->assets()->MakeDirty(actor->scene()->assetIdHash());
}

void SceneEditorWindow::m_DrawTransfotmButton(ShaderResource* icon, ImVec2 size, ImGuizmo::OPERATION target) {
	auto isSelected = m_CurrentGizmoOperation == target;
	if (isSelected)
		ImGui::PushStyleColor(ImGuiCol_Button, m_selectedColor);

	if (ImGui::ImageButton(icon->get(), size))
		m_CurrentGizmoOperation = target;

	if (isSelected)
		ImGui::PopStyleColor();
}

void SceneEditorWindow::m_DrawAudioVolume(ImVec2 size) {
	bool isMuted = scene()->audio()->IsMuted();
	float volume = scene()->audio()->volume();

	auto* icon = isMuted ? &game()->ui()->icSoundOff : &game()->ui()->icSoundOn;

	ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, { 0.0f, 0.0f });
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0.0f, 0.0f });
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

	if (ImGui::ImageButton(icon->get(), size)) {
		if (isMuted)
			scene()->audio()->Unmute();
		else
			scene()->audio()->Mute();
	}

	ImGui::PopStyleColor(1);
	ImGui::PopStyleVar(2);

	ImGui::SameLine();
	ImGui::SetNextItemWidth(60);
	if(ImGui::SliderFloat("##audio_volume", &volume, 0, 1, ""))
		scene()->audio()->volume(volume);
}

void SceneEditorWindow::m_ToggleGizmoMode() {
	m_CurrentGizmoMode = m_CurrentGizmoMode == ImGuizmo::LOCAL ? ImGuizmo::WORLD : ImGuizmo::LOCAL;
}

void SceneEditorWindow::m_HandleToolsInput() {
	if (game()->hotkeys()->GetButtonDownEd(Keys::Q))
		m_ToggleGizmoMode();

	if (game()->hotkeys()->GetButtonDownEd(Keys::E))
		m_CurrentGizmoOperation = ImGuizmo::TRANSLATE;

	if (game()->hotkeys()->GetButtonDownEd(Keys::R))
		m_CurrentGizmoOperation = ImGuizmo::ROTATE;

	if (game()->hotkeys()->GetButtonDownEd(Keys::T))
		m_CurrentGizmoOperation = ImGuizmo::SCALE;

	//if (game()->hotkeys()->GetButtonDown(Keys::L))
	//	m_CurrentGizmoOperation = ImGuizmo::BOUNDS;

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
