#include "UI_AssetsStore.h"
#include "UserInterface.h"
#include "AssetPickerPopup.h"

#include "../Game.h"
#include "../AssetStore.h"
#include "../imgui/misc/cpp/imgui_stdlib.h"

#include <iostream>
#include <algorithm>
#include <math.h>
#include <vector>

void UI_AssetsStore::Init(Game* game)
{
	m_game = game;
}

void UI_AssetsStore::DrawAssetsStore()
{
	if (ImGui::Begin("Assets Store"))
	{
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);

		float height = ImGui::GetFrameHeight();

		ImGui::PushStyleColor(ImGuiCol_Button, { 0, 0, 0, 0 });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0, 0, 0, 0 });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0, 0, 0, 0 });
		ImGui::ImageButton(m_game->ui()->icSearch.get(), { height, height }, { 0, 0 }, { 1, 1 }, 0, { 0,0,0,0 }, { 1,1,1,0.8f });
		ImGui::PopStyleColor(3);

		ImGui::SameLine();

		ImGui::InputText("##findType", &m_input);

		bool isInputDeactivated = ImGui::IsItemDeactivated();

		ImGui::Separator();
		ImGui::PopStyleVar(1);

		auto store = m_game->assetStore();

		ImGuiTreeNodeFlags header = 0
			| ImGuiTreeNodeFlags_Framed
			;

		auto filteredHeader = std::vector<int>();
		filteredHeader.reserve(store->assetTypes.size());

		auto node_flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

		for (int i = 0; i < store->assetTypes.size(); ++i)
		{
			auto typeId = store->assetTypes.at(i);
			auto typeName = store->GetScriptName(typeId);

			const std::vector<int>* assets = nullptr;

			if (store->assets.contains(typeId))
			{
				header |= ImGuiTreeNodeFlags_DefaultOpen;
				assets = &store->assets[typeId];
			}
			
			if (ImGui::CollapsingHeader(typeName.c_str(), header))
			{
				ImGui::Indent(25);
				for (int i = 0; i < assets->size(); ++i)
				{
					auto assetId = assets->at(i);
					auto assetName = store->GetAssetName(assetId);

					bool isMatched = (m_input == "") ? true : IsMatch(assetName, m_input);

					std::string selectableId = assetName + "##" + std::to_string(assetId);
					if (isMatched)
					{
						bool isSelected = assetId == m_popupOpenAssetId;
						ImGui::Selectable(selectableId.c_str(), isSelected);
						m_AssetsStoreContextMenu(assetId);
					}
				}
				ImGui::Unindent(25);
			}
		}
		ImGui::End();
	}
}

void UI_AssetsStore::m_AssetsStoreContextMenu(int assetId)
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 10.0f, 10.0f });
	ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 10.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0.0f, 3.0f });
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 5.0f, 5.0f });

	ImGui::PushStyleColor(ImGuiCol_PopupBg, { 0.7f, 0.7f, 0.7f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_Text, { 0.0f, 0.0f, 0.0f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_HeaderHovered, { 0.8f, 0.8f, 0.9f, 1.0f });

	bool isOpened = false;

	if (ImGui::BeginPopupContextItem(0, ImGuiPopupFlags_MouseButtonRight))
	{
		isOpened = true;
		m_popupOpenAssetId = assetId;

		if (ImGui::Selectable("Create"));
		{
			/*
				
			*/
			//m_sendAssetIdForCreate(assetId);
		}

		if (ImGui::Selectable("Rename"))
		{
			//m_sendAssetIdForRename(assetId, "New Name");
		}

		if (ImGui::Selectable("Remove"))
		{
			//m_sendAssetIdForRemove(assetId);
		}

		ImGui::EndPopup();
	}
	ImGui::PopStyleVar(6);
	ImGui::PopStyleColor(3);

	if (!isOpened && assetId == m_popupOpenAssetId)
		m_popupOpenAssetId = 0;
}

bool UI_AssetsStore::IsMatch(std::string source, std::string target)
{
	std::transform(source.begin(), source.end(), source.begin(), ::tolower);
	std::transform(target.begin(), target.end(), target.begin(), ::tolower);

	auto res = source.find(target);

	return res != std::string::npos;
}

void UI_AssetsStore::m_sendAssetIdForCreate(int assetID) {
	//m_game->ui()->_callbacks.sendAssetIdForCreate(csRef, assetID);
}

void UI_AssetsStore::m_sendAssetIdForRename(int assetID, std::string newName) {
	//m_game->ui()->_callbacks.sendAssetIdForRename(csRef, assetID, newName);
}

void UI_AssetsStore::m_sendAssetIdForRemove(int assetID) {
	//m_game->ui()->_callbacks.sendAssetIdForRemove(csRef, assetID);
}