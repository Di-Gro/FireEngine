#include "UI_Hierarchy.h"
#include "../Game.h"
#include "UserInterface.h"

void UI_Hierarchy::Draw_UI_Hierarchy()
{
	if (ImGui::Begin("Hierarchy"))
	{
		auto it = _game->GetNextRootActors(_game->BeginActor());
		for (; it != _game->EndActor(); it = _game->GetNextRootActors(++it))
		{
			VisitActor(*it);
		}
	}
	ImGui::End();
}

void UI_Hierarchy::Init(Game* game)
{
	_game = game;
}

void UI_Hierarchy::InitUI(UserInterface* ui)
{
	_ui = ui;
}

void UI_Hierarchy::VisitActor(Actor* actor)
{

	if (actor->GetChildrenCount() > 0)
	{
		//auto flags = (() ? (ImGuiTreeNodeFlags_Selected : 0))
		bool selectedTree = ImGui::TreeNodeEx(actor->GetName().c_str(), ImGuiTreeNodeFlags_OpenOnArrow);

		if (ImGui::IsItemClicked())
		{
			_ui->SelectedActor(actor);
			_ui->SetActorActive();
		}

		if (selectedTree)
		{
			for (int i = 0; i < actor->GetChildrenCount(); ++i)
			{
				VisitActor(actor->GetChild(i));
			}

			ImGui::TreePop();
		}
	}
	else
	{
		ImGui::Indent();
		std::string id = std::to_string(actor->Id());
		auto name = actor->GetName() == "" ? "Empty" : actor->GetName();
		//ImGui::Selectable((actor->GetName() + id).c_str());
		ImGui::Selectable(name.c_str());

		if (ImGui::IsItemClicked() && ImGui::IsItemActivated())
		{
			_ui->SelectedActor(actor);
			//_ui->SetActorActive();
			/*int n_next = n + (ImGui::GetMouseDragDelta(0).y < 0.f ? -1 : 1);
			if (n_next >= 0 && n_next < IM_ARRAYSIZE(item_names))
			{
				item_names[n] = item_names[n_next];
				item_names[n_next] = item;
				ImGui::ResetMouseDragDelta();
			}*/
		}
		ImGui::Unindent();
	}
}