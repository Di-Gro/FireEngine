#include "ScenePickerPopup.h"

#include <iostream>
#include <algorithm>
#include <math.h>

#include "../Game.h"
#include "../Scene.h"
#include "../Math.h"
#include "../AssetStore.h"
#include "../ContextMenu.h"
#include "ComponentPicker.h"

#include "../imgui/misc/cpp/imgui_stdlib.h"


bool ScenePickerPopup::IsMatch(std::string source, std::string target) {
	std::transform(source.begin(), source.end(), source.begin(), ::tolower);
	std::transform(target.begin(), target.end(), target.begin(), ::tolower);

	auto res = source.find(target);

	return res != std::string::npos;
}

bool ScenePickerPopup::Open(Scene* scene, const char* id) {
	auto game = scene->game();
	auto store = game->assetStore();

	m_typeIdHash = store->sceneTypeIdHash;

	if (!store->assets.contains(m_typeIdHash))
		return false;

	const auto* content = &store->assets[m_typeIdHash];

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

		hasSelected = m_DrawMenu(scene);
		
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


bool ScenePickerPopup::m_DrawMenu(Scene* scene) {
	auto game = scene->game();
	bool hasItems = false;
	bool hasUniqueName = m_filteredCount == 0 && m_input.size() > 0;
	auto filename = m_input;

	if (hasUniqueName) {
		if (SceneMenu::CanCreate(game, filename)) {

			if (ImGui::Selectable("Create")) {
				int assetIdHash = SceneMenu::Create(game, filename);
				if(assetIdHash != 0)
					selected = assetIdHash;

				ImGui::CloseCurrentPopup();
				return true;
			}
			hasItems = true;
		}
		if (SceneMenu::CanRename(scene, filename)) {

			if (ImGui::Selectable("Rename")) {
				SceneMenu::Rename(scene, filename);
				ImGui::CloseCurrentPopup();
			}
			hasItems = true;
		}
	}

	if (SceneMenu::CanSave(scene)) {
		if (ImGui::Selectable("Save")) {
			SceneMenu::Save(scene);
			ImGui::CloseCurrentPopup();
		}
		hasItems = true;
	}
	if (SceneMenu::CanSetAsStartup(scene)) {

		if (ImGui::Selectable("Set as startup")) {
			SceneMenu::SetAsStartup(scene);
			ImGui::CloseCurrentPopup();
		}
		hasItems = true;
	}

	if (hasItems)
		ImGui::Separator();

	return false;
}