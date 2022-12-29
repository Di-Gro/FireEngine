#include "UI_Console.h"
#include "../Game.h"

void UI_Console::Draw_UI_Console()
{
	if (ImGui::Begin("Console"))
	{
		DrawConsoleButtons();

		
	}ImGui::End();
}

void UI_Console::DrawConsoleButtons()
{
	/*if (ImGui::Begin("##consoleButtons", NULL, ImGuiWindowFlags_NoTitleBar))
	{

		ImGui::End();
	}*/
}

void UI_Console::DrawConsoleInformation()
{

}