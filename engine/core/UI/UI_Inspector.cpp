#include "UI_Inspector.h"
#include "../Game.h"
#include "UserInterface.h"
#include "../imgui/imgui_internal.h"
#include <list>

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

void UI_Inspector::Draw_UI_Inspector()
{
	if (ImGui::Begin("Inspector"))
	{
		if (_ui->HasActor())
		{
			if (ImGui::TreeNodeEx("Transform", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow))
			{
				ImGui::Separator();
				DrawActorTransform();
				ImGui::TreePop();
			}

			DrawActorComponents();

			/*ImGui::Separator();
			if (ImGui::Button("Add Component"))
			{

			}*/
		}
		else
		{
			ImGui::Text("Actor is not get!");
		}
		
	}ImGui::End();
}

void UI_Inspector::DrawActorTransform()
{
	auto wp = _ui->GetActor()->localPosition();
	auto lastWp = wp;

	auto wr = _ui->GetActor()->localRotation();
	auto lastWr = wr;

	auto ws = _ui->GetActor()->localScale();
	auto lastWs = ws;

	ShowVector3(&wp, "Position");
	if (wp != lastWp)
		_ui->GetActor()->localPosition(wp);

	ShowVector3(&wr, "Rotation");
	if (wr != lastWr)
		_ui->GetActor()->localRotation(wr);

	ShowVector3(&ws, "Scale");
	if (ws != lastWs)
		_ui->GetActor()->localScale(ws);
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

void UI_Inspector::Init(Game* game)
{
	_game = game;
}

void UI_Inspector::InitUI(UserInterface* ui)
{
	_ui = ui;
}