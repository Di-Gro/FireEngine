#include "AssetPickerPopup.h"

#include <iostream>
#include <algorithm>
#include <math.h>

#include "../Game.h"
#include "../Math.h"
#include "../AssetStore.h"
#include "ComponentPicker.h"

#include "../imgui/misc/cpp/imgui_stdlib.h"

#include "../AssetIterator.h"


bool AssetPickerPopup::IsMatch(std::string source, std::string target) {
	std::transform(source.begin(), source.end(), source.begin(), ::tolower);
	std::transform(target.begin(), target.end(), target.begin(), ::tolower);

	auto res = source.find(target);

	return res != std::string::npos;
}

bool AssetPickerPopup::Open(Game* game) {
	auto store = game->assetStore();

	if (scriptIdHash == 0)
		return false;

	auto selection = AssetSelection(store->assets);

	if (scriptIdHash == store->anyScriptIdHash) {
		selection.Exclude(store->sceneTypeIdHash);
		selection.Exclude(store->actorTypeIdHash);
	}
	else {
		selection.Include(scriptIdHash);
	}

	bool hasSelected = false;

	ComponentPicker::PushPopupStyles();

	//auto filtered = std::vector<int>();
	//filtered.reserve(content->size());

	auto lastOpen = m_isOpen;
	auto lastSelected = m_selectedIndex;
	auto lastSize = m_size;

	m_isOpen = false;
	m_selectedIndex = 0;

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

		//if (filtered.size() == 1)
		//	m_selectedIndex = 0;

		if (ImGui::IsKeyPressed(ImGuiKey_Enter) && isInputDeactivated) {
			if (lastSelected != 0) {
				selected = lastSelected;
				hasSelected = true;
			}
			ImGui::CloseCurrentPopup();
		}

		float size = lastSize * (textSize.y + ImGui::GetStyle().ItemSpacing.y);
		float child_w = ImGui::GetContentRegionAvail().x;
		float child_h = clamp(size, 1, 250);
		const bool child_is_visible = ImGui::BeginChild("##scrol", ImVec2(child_w, child_h), false, 0);

		if (child_is_visible) {
			m_size = 0;
			for (auto assetHash : selection) {
				auto assetName = store->GetAssetName(assetHash);
				if (!IsMatch(assetName, m_input))
					continue;

				m_size++;

				bool isSelected = m_selectedIndex == assetHash;

				if (ImGui::Selectable((assetName + "##" + std::to_string(assetHash)).c_str(), isSelected)) {
					selected = assetHash;
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
