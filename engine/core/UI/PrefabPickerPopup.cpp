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

		hasSelected = m_DrawMenu(actor);
		
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


bool PrefabPickerPopup::m_DrawMenu(Actor* actor) {
	auto game = actor->game();
	bool hasItems = false;
	bool hasUniqueName = m_filteredCount == 0 && m_input.size() > 0;
	auto filename = m_input + ".yml";

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
		{
			if (ImGui::Selectable("Null")) {
				selected = 0;
				ImGui::CloseCurrentPopup();
				return true;
			}
			hasItems = true;
		}
		if (PrefabMenu::CanSave(actor)) {

			if (ImGui::Selectable("Save")) {
				PrefabMenu::Save(actor);
				ImGui::CloseCurrentPopup();
			}
			hasItems = true;
		}
		if (PrefabMenu::CanLoad(actor)) {

			if (ImGui::Selectable("Load")) {
				PrefabMenu::Load(actor);
				ImGui::CloseCurrentPopup();
			}
			hasItems = true;
		}
	}
	if(hasItems)
		ImGui::Separator();

	return false;
}