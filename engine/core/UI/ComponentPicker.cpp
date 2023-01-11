#include "ComponentPicker.h"

#include <iostream>
#include <algorithm>
#include <math.h>

#include "../Math.h"
#include "../Game.h"
#include "../AssetStore.h"
#include "UI_Hierarchy.h"

#include "../imgui/misc/cpp/imgui_stdlib.h"


bool ComponentPicker::IsMatch(std::string source, std::string target) {
	std::transform(source.begin(), source.end(), source.begin(), ::tolower);
	std::transform(target.begin(), target.end(), target.begin(), ::tolower);

	auto res = source.find(target);

	return res != std::string::npos;
}

void ComponentPicker::PushPopupStyles() {
	ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 10.0f, 10.0f });
	ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 10.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0.0f, 3.0f });
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 5.0f, 5.0f });

	//ImGui::PushStyleColor(ImGuiCol_ScrollbarBg, { 0.7f, 0.7f ,0.7f ,1.0f });
	//ImGui::PushStyleColor(ImGuiCol_ChildBg, { 0.7f, 0.7f ,0.7f ,1.0f });
	//ImGui::PushStyleColor(ImGuiCol_PopupBg, { 0.7f, 0.7f ,0.7f ,1.0f });
	//ImGui::PushStyleColor(ImGuiCol_Text, { 0.0f, 0.0f ,0.0f ,1.0f });
	//ImGui::PushStyleColor(ImGuiCol_Header, { 0.8f, 0.8f ,0.9f ,1.0f });
	//ImGui::PushStyleColor(ImGuiCol_HeaderHovered, { 0.8f, 0.8f ,0.9f ,1.0f });

	//ImGui::PushStyleColor(ImGuiCol_FrameBg, { 0.5f, 0.5f, 0.5f, 1.0f });
	//ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, { 0.5f, 0.5f, 0.5f, 1.0f });
	//ImGui::PushStyleColor(ImGuiCol_FrameBgActive, { 0.5f, 0.5f, 0.5f, 1.0f });

	ImGui::PushStyleColor(ImGuiCol_PopupBg, { 0.2f, 0.2f ,0.2f ,1.0f });
	ImGui::PushStyleColor(ImGuiCol_Text, { 0.8f, 0.8f ,0.8f ,1.0f });
	ImGui::PushStyleColor(ImGuiCol_Header, { 0.4f, 0.4f ,0.4f ,1.0f });
	ImGui::PushStyleColor(ImGuiCol_HeaderHovered, { 0.4f, 0.4f ,0.4f ,1.0f });
	ImGui::PushStyleColor(ImGuiCol_ScrollbarBg, { 0.2f, 0.2f ,0.2f ,1.0f });
	ImGui::PushStyleColor(ImGuiCol_ChildBg, { 0.2f, 0.2f ,0.2f ,1.0f });
}

void ComponentPicker::PopPopupStyles() {
	ImGui::PopStyleVar(7);
	//ImGui::PopStyleColor(9);
	ImGui::PopStyleColor(6);
}

bool ComponentPicker::Open(Game* game) {
	auto store = game->assetStore();

	const auto* content = &store->components;

	bool hasSelected = false;

	PushPopupStyles();

	auto filtered = std::vector<int>();
	filtered.reserve(content->size());

	auto lastOpen = m_isOpen;
	auto lastSelected = m_selectedIndex;

	m_isOpen = false;
	m_selectedIndex = -1;

	if (ImGui::BeginPopupContextWindow(0, flags)) {

		m_isOpen = true;

		auto headerText = header.c_str();
		auto windowWidth = ImGui::GetWindowSize().x;
		auto textSize = ImGui::CalcTextSize(headerText);

		ImGui::SetCursorPosX((windowWidth - textSize.x) * 0.5f);
		ImGui::Text(headerText);

		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
		ImGui::InputText("##findType", &m_input);

		if (!ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
			ImGui::SetKeyboardFocusHere(0);

		bool isInputDeactivated = ImGui::IsItemDeactivated();

		ImGui::Separator();
		ImGui::PopStyleVar(1);

		for (int i = 0; i < content->size(); i++) {
			auto name = store->GetScriptName(content->at(i));
			if (IsMatch(name, m_input))
				filtered.push_back(i);
		}

		if (filtered.size() == 1)
			m_selectedIndex = 0;

		if (ImGui::IsKeyPressed(ImGuiKey_Enter) && isInputDeactivated) {
			if (lastSelected >= 0) {
				int index = filtered[lastSelected];
				selected = store->GetScriptFullName(content->at(index));
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
				auto name = store->GetScriptName(content->at(i));

				if (ImGui::Selectable((name + "##" + std::to_string(i)).c_str(), isSelected)) {
					selected = store->GetScriptFullName(content->at(i));;
					ImGui::CloseCurrentPopup();
					hasSelected = true;
				}
			}
		}
		ImGui::EndChild();

		ImGui::EndPopup();
	}
	PopPopupStyles();
	

	return hasSelected;
}