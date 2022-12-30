#include "UI_Hierarchy.h"

#include "../Game.h"
#include "../Scene.h"
#include "UserInterface.h"
#include "../Game.h"

void UI_Hierarchy::Draw_UI_Hierarchy()
{
	auto scene = _game->ui()->selectedScene();
	if (scene == nullptr)
		return;

	int counter = 0;
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

	if (ImGui::Begin("Hierarchy"))
	{
		auto it = scene->GetNextRootActors(scene->BeginActor());
		for (; it != scene->EndActor(); it = scene->GetNextRootActors(++it))
		{
			VisitActor(*it, counter);
			counter++;
		}
	}
	ImGui::End();
	ImGui::PopStyleVar();
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

	if (actor->GetChildrenCount() > 0)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, treeNodeHeight));
		bool selectedTree = ImGui::TreeNodeEx(actor->GetName().c_str(), node_flags);
		ImGui::PopStyleVar();

		if (ImGui::IsItemClicked())
		{
			_ui->SelectedActor(actor);
			_ui->SetActorActive();
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

		auto name = actor->GetName() == "" ? "Empty" : actor->GetName();

		node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, treeNodeHeight));
		ImGui::TreeNodeEx(name.c_str(), node_flags);
		ImGui::PopStyleVar();

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
