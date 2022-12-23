#include "UI_BarMenu.h"
#include "../Game.h"

void UI_BarMenu::Draw_UI_BarMenu()
{
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			ImGui::MenuItem("New", "CTRL + N");
			ImGui::MenuItem("Open", "CTRL + O");
			ImGui::MenuItem("Save", "CTRL + S");
			ImGui::MenuItem("Save as", NULL);
			ImGui::Separator();

			if (ImGui::MenuItem("Exit"))
			{
				_game->Exit(0);
			};

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Options"))
		{
			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}
}

void UI_BarMenu::Init(Game* game)
{
	_game = game;
}