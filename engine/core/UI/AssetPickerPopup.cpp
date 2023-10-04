#include "AssetPickerPopup.h"

#include <iostream>
#include <algorithm>
#include <math.h>

#include "../Game.h"
#include "../Math.h"
#include "../AssetStore.h"
#include "ComponentPicker.h"

#include "../imgui/misc/cpp/imgui_stdlib.h"

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

	if (!store->assets.contains(scriptIdHash))
		return false;

	//const auto* content = &store->assets[scriptIdHash];
	const std::vector<int>* content = &m_emptyContent;
	if (store->assets.contains(scriptIdHash))
		content = &store->assets[scriptIdHash];

	bool hasSelected = false;

	//ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 3.0f, 3.0f });
	//ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 6.0f);
	//ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, 1.0f);
	//ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
	//ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0.0f, 3.0f });
	//ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
	//ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 5.0f, 5.0f });

	//ImGui::PushStyleColor(ImGuiCol_PopupBg, { 0.2f, 0.2f ,0.2f ,1.0f });
	//ImGui::PushStyleColor(ImGuiCol_Text, { 0.8f, 0.8f ,0.8f ,1.0f });
	//ImGui::PushStyleColor(ImGuiCol_Header, { 0.4f, 0.4f ,0.4f ,1.0f });
	//ImGui::PushStyleColor(ImGuiCol_HeaderHovered, { 0.4f, 0.4f ,0.4f ,1.0f });
	//ImGui::PushStyleColor(ImGuiCol_ScrollbarBg, { 0.2f, 0.2f ,0.2f ,1.0f });
	//ImGui::PushStyleColor(ImGuiCol_ChildBg, { 0.2f, 0.2f ,0.2f ,1.0f });
	ComponentPicker::PushPopupStyles();

	auto filtered = std::vector<int>();
	filtered.reserve(content->size());

	auto lastOpen = m_isOpen;
	auto lastSelected = m_selectedIndex;

	m_isOpen = false;
	m_selectedIndex = -1;

	//ImGui::PushID(imguiID);
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

		for (int i = 0; i < content->size(); i++) {
			if (IsMatch(store->GetAssetName(content->at(i)), m_input))
				filtered.push_back(i);
		}

		if (filtered.size() == 1)
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
	//ImGui::PopID();

	//ImGui::PopStyleVar(7);
	//ImGui::PopStyleColor(6);
	ComponentPicker::PopPopupStyles();

	return hasSelected;
}
