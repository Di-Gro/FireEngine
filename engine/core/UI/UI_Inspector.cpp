#include "UI_Inspector.h"
#include "UserInterface.h"
#include "../Game.h"
#include "../imgui/imgui_internal.h"
#include <list>
#include <string>

char UI_Inspector::textBuffer[1024] = { 0 };

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
			if (ImGui::TreeNodeEx("Transform", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick))
			{
				ImGui::Separator();
				DrawActorTransform();
				ImGui::TreePop();
			}

			DrawActorComponents();


			ImGui::Separator();
			AddComponent();
		}
		
	}ImGui::End();
}

void UI_Inspector::AddComponent()
{
	bool isActiveAddComponent = false;
	std::string searchText = "";
	if (ButtonCenteredOnLine("Add Component", 0.5f))
	{
		if (!isActiveAddComponent)
			isActiveAddComponent = true;
	}

	if (isActiveAddComponent)
	{
		static char str0[128] = "Hello, world!";
		ImGui::InputText("##Search", str0, IM_ARRAYSIZE(str0));
		ImGui::Text(str0);
	}
}

void UI_Inspector::DrawActorTransform()
{
	auto wp = _ui->GetActor()->localPosition();
	auto wr = _ui->GetActor()->localRotation();
	auto ws = _ui->GetActor()->localScale();

	if(ShowVector3(&wp, "Position"))
		_ui->GetActor()->localPosition(wp);

	if (ShowVector3(&wr, "Rotation"))
		_ui->GetActor()->localPosition(wr);

	if (ShowVector3(&ws, "Scale"))
		_ui->GetActor()->localPosition(ws);
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
			if (ImGui::TreeNodeEx("Component", ImGuiTreeNodeFlags_DefaultOpen))
			{
				widthComponent = ImGui::GetContentRegionAvail().x;
				ImGui::Separator();
				tmp(_csRef, widthComponent);
				ImGui::TreePop();
			}
			break;
		}
	}
}

bool UI_Inspector::ShowVector3(Vector3* values, const std::string& title)
{
	auto tmpValues = *values;

	ImGui::Columns(2, nullptr, false);
	ImGui::SetColumnWidth(0, 80.0f);
	ImGui::Text(title.c_str());
	ImGui::NextColumn();
	ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - 50.0f);

	std::string nameX = "##X_" + title;
	std::string nameY = "##Y_" + title;
	std::string nameZ = "##Z_" + title;

	ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, 5.0f });

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

	return tmpValues != *values;
}

void UI_Inspector::Init(Game* game)
{
	_game = game;
}

void UI_Inspector::InitUI(UserInterface* ui)
{
	_ui = ui;
}

DEF_FUNC(UI_Inspector, ShowText, bool)(CppRef gameRef, const char* label, const char* buffer, int length, size_t* ptr)
{
	length = (length < 1024) ? length : 1024;
	std::memcpy(UI_Inspector::textBuffer, buffer, length);
	
	*ptr = (size_t)UI_Inspector::textBuffer;

	auto game = CppRefs::ThrowPointer<Game>(gameRef);

	std::string tmpLabel = "##" + (std::string)label;

	ImGui::Columns(2, "", false);
	ImGui::SetColumnWidth(0, 100.0f);

	ImGui::Text(label);

	ImGui::NextColumn();
	ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x + 7.0f);
	
	//ImGui::InputText(tmpLabel.c_str(), (char*)buffer, length + 1);
	ImGui::InputText(tmpLabel.c_str(), UI_Inspector::textBuffer, sizeof(UI_Inspector::textBuffer));

	ImGui::PopItemWidth();
	ImGui::Columns(1);

	return false;
}