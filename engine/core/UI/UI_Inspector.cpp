#include "UI_Inspector.h"

#include <list>

#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"
#include "../imguizmo/ImGuizmo.h"

#include "../Game.h"
#include "UserInterface.h"
#include "../Render.h"
#include "../HotKeys.h"

#include "../CameraComponent.h"




bool UI_Inspector::ButtonCenteredOnLine(const char* label, float alignment)
{
	ImGuiStyle& style = ImGui::GetStyle();

	float size = ImGui::CalcTextSize(label).x + style.FramePadding.x * 2.0f;
	float avail = ImGui::GetContentRegionAvail().x;

	float off = (avail - size) * alignment;
	if (off > 0.0f)
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);

	return ImGui::Button(label);
}

//void InspectorSeparator() {
//	ImGuiStyleVar_
//	ImGui::PushStyleVar()
//	ImGui::Separator();
//
//}

void UI_Inspector::BigSpace() {
	ImGui::Dummy(m_compSpacing);
}

void UI_Inspector::Space() {
	ImGui::Dummy(m_lineSpacing);
}

void UI_Inspector::m_DrawItem(void(UI_Inspector::*itemFunc)()) {
	BigSpace();
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 3.0f, 0.0f });

	(this->*itemFunc)();

	ImGui::PopStyleVar(2);
	BigSpace();
	ImGui::Separator();
}

void UI_Inspector::Draw_UI_Inspector() {

	auto treeNodeFlags = 0
		| ImGuiTreeNodeFlags_DefaultOpen
		| ImGuiTreeNodeFlags_OpenOnArrow
		| ImGuiTreeNodeFlags_OpenOnDoubleClick
		| ImGuiTreeNodeFlags_Framed
		//| ImGuiTreeNodeFlags_SpanFullWidth
		;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0.0f, 3.0f });
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0.0f, 0.0f });

	if (ImGui::Begin("Inspector")) {
		if (_ui->HasActor()) {

			m_DrawItem(&UI_Inspector::m_DrawHeader);
									
			if (ImGui::TreeNodeEx("Transform", treeNodeFlags)) {
				m_DrawItem(&UI_Inspector::DrawActorTransform);
				ImGui::TreePop();
			}
			DrawActorComponents();

			/*ImGui::Separator();
			if (ImGui::Button("Add Component"))
			{

			}*/

		}
		else {
			ImGui::Text("Actor is not get!");
		}
		
	}
	ImGui::End();
	ImGui::PopStyleVar(4);
}

void UI_Inspector::DrawActorTransform()
{
	auto actor = _ui->GetActor();
	auto matrix = (ImGuizmo::matrix_t&)actor->GetLocalMatrix();

	float matrixTranslation[3], matrixRotation[3], matrixScale[3];
	ImGuizmo::DecomposeMatrixToComponents(matrix.m16, matrixTranslation, matrixRotation, matrixScale);

	ShowVector3((Vector3*)matrixTranslation, "Position");
	Space();
	ShowVector3((Vector3*)matrixRotation, "Rotation");
	Space();
	ShowVector3((Vector3*)matrixScale, "Scale");

	ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, matrix.m16);
	actor->SetLocalMatrix((Matrix)matrix);
}

void UI_Inspector::m_DrawHeader() {

	auto actor = _game->ui()->GetActor();
	
	const auto bufSize = 80;
	char nameBuffer[bufSize] = { 0 };
	auto name = actor->GetName();
	std::memcpy(&nameBuffer, name.c_str(), min(name.size(), bufSize));

	bool isActive = true;
	auto id = "ID: " + std::to_string(actor->Id());

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 10.0f, 0.0f });
		
	ImGui::Indent(10);
	ImGui::Checkbox("##ObjectIsActive", &isActive);

	ImGui::SameLine();
	ImGui::Text(id.c_str());
 
	ImGui::SameLine();
	ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - 10);
	ImGui::InputText("##ObjectName", nameBuffer, bufSize);
	bool textActive = ImGui::IsItemActive();

	ImGui::PopStyleVar(1);
	ImGui::Unindent(10);

	if (textActive && _game->hotkeys()->GetButtonDown(Keys::Enter)) {
		//TODO: установить измененное имя. 
		//TODO: как стирать символы?
	}
}

void UI_Inspector::DrawActorComponents()
{
	std::vector<size_t> componentRefs;
	componentRefs.resize(_ui->GetActor()->GetComponentsCount());

	auto list = _ui->GetActor()->GetComponentList();

	auto tmp = _ui->_callbacks.onDrawComponent;

	for (auto component : *list) {
		auto _csRef = component->csRef();
		if (!component->IsDestroyed() && _csRef.value > 0) {
			tmp(_csRef);
		}
	}
}

void UI_Inspector::ShowVector3(Vector3* values, const std::string& title)
{
	ImGui::Columns(2, nullptr, false);
	ImGui::SetColumnWidth(0, 80.0f);
	ImGui::Text(title.c_str());
	ImGui::NextColumn();
	ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - 50.0f);

	std::string nameX = "##X_" + title;
	std::string nameY = "##Y_" + title;
	std::string nameZ = "##Z_" + title;

	ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, 0 });

	ImGui::Text(" X ");
	ImGui::SameLine();
	ImGui::DragFloat(nameX.c_str(), &values->x, 0.1f);
	ImGui::PopItemWidth();
	ImGui::SameLine();

	ImGui::Text(" Y ");
	ImGui::SameLine();
	ImGui::DragFloat(nameY.c_str(), &values->y, 0.1f);
	ImGui::PopItemWidth();
	ImGui::SameLine();

	ImGui::Text(" Z ");
	ImGui::SameLine();
	ImGui::DragFloat(nameZ.c_str(), &values->z, 0.1f);
	ImGui::PopItemWidth();

	ImGui::PopStyleVar();

	ImGui::Columns(1);
}

void UI_Inspector::Init(Game* game, UserInterface* ui)
{
	_game = game;
	_ui = ui;
}
