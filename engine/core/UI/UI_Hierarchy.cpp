#include "UI_Hierarchy.h"

#include "../Game.h"
#include "../Scene.h"
#include "UserInterface.h"
#include "../Game.h"
#include "../Actor.h"
#include "../Lighting.h"
#include "../DirectionLight.h"
#include "../AmbientLight.h"
#include "../LinedPlain.h"

void UI_Hierarchy::Draw_UI_Hierarchy() {
	int counter = 0;
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

	if (ImGui::Begin("Hierarchy")) {
		auto scene = _game->ui()->selectedScene();
		if (scene != nullptr) {
			_game->PushScene(scene);

			m_DrawSceneContextMenu();
			auto it = scene->GetNextRootActors(scene->BeginActor());
			for (; it != scene->EndActor(); it = scene->GetNextRootActors(++it))
			{
				VisitActor(*it, counter);
				counter++;
			}

			_game->PopScene();
		}
	}
	ImGui::End();
	ImGui::PopStyleVar();

}

void UI_Hierarchy::m_PushPopupStyles() {
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 10.0f, 10.0f });
	ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 10.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0.0f, 3.0f });
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 5.0f, 5.0f });

	ImGui::PushStyleColor(ImGuiCol_PopupBg, { 0.7f, 0.7f ,0.7f ,1.0f });
	ImGui::PushStyleColor(ImGuiCol_Text, { 0.0f, 0.0f ,0.0f ,1.0f });
	ImGui::PushStyleColor(ImGuiCol_HeaderHovered, { 0.8f, 0.8f ,0.9f ,1.0f });
}

void UI_Hierarchy::m_PopPopupStyles() {
	ImGui::PopStyleVar(6);
	ImGui::PopStyleColor(3);
}

void UI_Hierarchy::m_DrawSceneContextMenu() {
	m_PushPopupStyles();

	auto flags = ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems;
	if (ImGui::BeginPopupContextWindow(0, flags))
	{
		if (ImGui::Selectable("Add Actor")) {
			auto actor = _game->currentScene()->CreateActor();
		}

		bool canAddLight = 
				_game->currentScene()->directionLight == nullptr
			||	_game->currentScene()->ambientLight == nullptr;

		auto lightFlags = !canAddLight ? ImGuiSelectableFlags_Disabled : 0;

		if (ImGui::Selectable("Add Light", false, lightFlags)) {
			auto actor = _game->currentScene()->CreateActor();
			actor->name("Light");

			actor->localPosition({ 0, 0, 300 });
			actor->localRotation({ rad(-45), rad(45 + 180), 0 });

			if (_game->currentScene()->directionLight == nullptr)
				actor->AddComponent<DirectionLight>();

			if (_game->currentScene()->ambientLight == nullptr)
				actor->AddComponent<AmbientLight>();
		}

		ImGui::EndPopup();
	}

	m_PopPopupStyles();
}

void UI_Hierarchy::m_DrawActorContextMenu(Actor* actor) {
	m_PushPopupStyles();

	auto flags = ImGuiPopupFlags_MouseButtonRight;
	if (ImGui::BeginPopupContextItem(0, flags)) {
		if (ImGui::Selectable("Add Child")) {
			auto child = _game->currentScene()->CreateActor(actor);
		}
		if (ImGui::Selectable("Remove")) {
			if (_game->ui()->GetActor() == actor)
				_game->ui()->SelectedActor(nullptr);
			actor->Destroy();
		}
		ImGui::EndPopup();
	}
	m_PopPopupStyles();
}

void UI_Hierarchy::Init(Game* game)
{
	_game = game;
	_ui = _game->ui();
}

void UI_Hierarchy::VisitActor(Actor* actor, int counter)
{
	ImGuiTreeNodeFlags node_flags = (actor == _ui->GetActor() ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_FramePadding;
	float treeNodeHeight = 3.0f;
	static bool test_drag_and_drop = false;

	std::string actorId = std::to_string(actor->Id());
	auto treeNodeId = actor->name() + "##" + actorId + "SceneTreeNodeEx";

	if (actor->GetChildrenCount() > 0)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, treeNodeHeight));
		bool selectedTree = ImGui::TreeNodeEx(treeNodeId.c_str(), node_flags);
		ImGui::PopStyleVar();

		m_DrawActorContextMenu(actor);
		
		if (ImGui::IsItemClicked())
		{
			_ui->SelectedActor(actor);
			//_ui->SetActorActive();
			test_drag_and_drop = true;
		}

		if (test_drag_and_drop && ImGui::BeginDragDropSource())
		{
			ImGui::SetDragDropPayload("_TREENODE", NULL, 0);
			ImGui::Text("This is a drag and drop source");
			ImGui::EndDragDropSource();
		}

		if (selectedTree)
		{
			for (int i = 0; i < actor->GetChildrenCount(); ++i)
			{
				VisitActor(actor->GetChild(i), counter);
			}

			ImGui::TreePop();
		}
	}
	else
	{
		ImGui::Indent();

		auto name = actor->name();
		if (name == "") 
			name = "Empty";

		node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, treeNodeHeight));
		ImGui::TreeNodeEx(treeNodeId.c_str(), node_flags);
		ImGui::PopStyleVar();

		m_DrawActorContextMenu(actor);

		if (ImGui::IsItemClicked() && ImGui::IsItemActivated())
		{
			_ui->SelectedActor(actor);
			test_drag_and_drop = true;
		}

		if (test_drag_and_drop && ImGui::BeginDragDropSource())
		{
			ImGui::SetDragDropPayload("_TREENODE", NULL, 0);
			ImGui::Text("This is a drag and drop source");
			ImGui::EndDragDropSource();
		}
		ImGui::Unindent();
	}
}
