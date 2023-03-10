#include "PrefabPickerPopup.h"

#include <iostream>
#include <algorithm>
#include <math.h>

#include "../Game.h"
#include "../Actor.h"
#include "../Math.h"
#include "../AssetStore.h"
#include "../ContextMenu.h"
#include "ComponentPicker.h"

#include "../imgui/misc/cpp/imgui_stdlib.h"


bool PrefabPickerPopup::IsMatch(std::string source, std::string target) {
	std::transform(source.begin(), source.end(), source.begin(), ::tolower);
	std::transform(target.begin(), target.end(), target.begin(), ::tolower);

	auto res = source.find(target);

	return res != std::string::npos;
}

bool PrefabPickerPopup::Open(Actor* actor) {
	auto game = actor->game();
	auto store = game->assetStore();

	m_prefabIdHash = store->prefabTypeIdHash;

	if (!store->assets.contains(m_prefabIdHash))
		return false;

	const auto* content = &store->assets[m_prefabIdHash];

	bool hasSelected = false;

	ComponentPicker::PushPopupStyles();

	auto filtered = std::vector<int>();
	filtered.reserve(content->size());

	auto lastOpen = m_isOpen;
	auto lastSelected = m_selectedIndex;

	m_isOpen = false;
	m_selectedIndex = -1;

	if (ImGui::BeginPopupContextItem(0, flags)) {

		m_isOpen = true;

		auto headerText = header.c_str();
		auto windowWidth = ImGui::GetWindowSize().x;
		auto textSize = ImGui::CalcTextSize(headerText);

		ImGui::SetCursorPosX((windowWidth - textSize.x) * 0.5f);
		ImGui::Text(headerText);

		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);

		ImGui::InputText("##findType", &m_input, ImGuiInputTextFlags_AutoSelectAll);

		if (!ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
			ImGui::SetKeyboardFocusHere(0);

		bool isInputDeactivated = ImGui::IsItemDeactivated();

		ImGui::Separator();
		ImGui::PopStyleVar(1);

		selected = -1;
		m_DrawMenu(actor);
		hasSelected = selected != -1;
		
		for (int i = 0; i < content->size(); i++) {
			if (IsMatch(store->GetAssetName(content->at(i)), m_input))
				filtered.push_back(i);
		}

		m_filteredCount = filtered.size();

		if (m_filteredCount == 1)
			m_selectedIndex = 0;

		if (ImGui::IsKeyPressed(ImGuiKey_Enter) && isInputDeactivated) {
			if (lastSelected >= 0) {
				selected = content->at(filtered[lastSelected]);
				hasSelected = true;
			}
			ImGui::CloseCurrentPopup();
		}

		float size = filtered.size() * (textSize.y + ImGui::GetStyle().ItemSpacing.y);
		float child_w = ImGui::GetContentRegionAvail().x;
		float child_h = clamp(size, 1, 250);
		const bool child_is_visible = ImGui::BeginChild("##scrol", ImVec2(child_w, child_h), false, 0);

		if (child_is_visible) {
			for (auto i : filtered) {
				bool isSelected = m_selectedIndex >= 0 && m_selectedIndex < filtered.size() ? filtered[m_selectedIndex] == i : false;

				if (ImGui::Selectable((store->GetAssetName(content->at(i)) + "##" + std::to_string(i)).c_str(), isSelected)) {
					selected = content->at(i);
					ImGui::CloseCurrentPopup();
					hasSelected = true;
				}
			}
		}
		ImGui::EndChild();

		ImGui::EndPopup();
	}
	ComponentPicker::PopPopupStyles();

	return hasSelected;
}


void PrefabPickerPopup::m_DrawMenu(Actor* actor) {
	auto game = actor->game();
	bool hasItems = false;
	bool hasUniqueName = m_filteredCount == 0 && m_input.size() > 0;
	auto filename = m_input;
	bool result = false;

	if (hasUniqueName) {
		if (PrefabMenu::CanCreate(actor, filename)) {

			if (ImGui::Selectable("Create")) {
				PrefabMenu::Create(actor, filename);
				ImGui::CloseCurrentPopup();
			}
			hasItems = true;
		}
	}
	else {
		if (actor->prefabId() != "") {
			MenuAction actions[3] = {
				{
					"Save",
					PrefabMenu::CanSave(actor),
					[actor]() { PrefabMenu::Save(actor); }
				},
				{
					"Load",
					PrefabMenu::CanLoad(actor),
					[actor]() { PrefabMenu::Load(actor); }
				},
				{
					"Set Null",
					true,
					[this]() { selected = 0; }
				}
			};

			m_DrawMenuActions(2, &actions[0]);
			m_DrawMenuActions(1, &actions[2]);

			//{
			//	int col = 0;
			//	ImGui::SetColumnWidth(col++, btWidth);
			//	if (canSave) ImGui::SetColumnWidth(col++, btWidth);
			//	if (canLoad) ImGui::SetColumnWidth(col++, btWidth);
			//}
			//if (canSave) {
			//	if (ImGui::Button("Save", { availWidth / 3 - 5, 0 })) {
			//		PrefabMenu::Save(actor);
			//		ImGui::CloseCurrentPopup();
			//	}
			//}

			//ImGui::NextColumn();

			//if (canLoad) {
			//	if (ImGui::Button("Load", { availWidth / 3 - 5, 0 })) {
			//		PrefabMenu::Load(actor);
			//		ImGui::CloseCurrentPopup();
			//	}
			//}

			//ImGui::NextColumn();

			//if (ImGui::Button("Set Null", { availWidth / 3 - 5, 0 })) {
			//	selected = 0;
			//	ImGui::CloseCurrentPopup();
			//	result = true;
			//}

			//ImGui::Columns(1);
			hasItems = true;
		}
	}
	if(hasItems)
		ImGui::Separator();
}

void PrefabPickerPopup::m_DrawMenuActions(int count, MenuAction* actions) {
	//int btCount = 0;
	//for (int i = 0; i < count; i++) {
	//	if(actions[i].condition)
	//		btCount++;
	//}
	//if (btCount == 0)
	//	return;

	//float availWidth = ImGui::GetContentRegionAvailWidth();
	float btPadding = 5;
	float availWidth = 207 + btPadding * count; // ImGui::GetWindowSize().x;
	float btWidth = 207 / count + 5;
	

	ImGui::Columns(count, nullptr, false);

	for (int column = 0; column < count; column++) {
		auto action = actions[column];

		if (count > 1)
			ImGui::SetColumnWidth(column, btWidth);

		if (!action.condition)
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);

		ImVec2 btSize = { btWidth - btPadding, 0 };

		if (ImGui::Button(action.label, btSize)) {
			action.action();
			ImGui::CloseCurrentPopup();
		}

		if (!action.condition)
			ImGui::PopItemFlag();

		if (column != count - 1)
			ImGui::NextColumn();
	}

	ImGui::Columns(1);
}