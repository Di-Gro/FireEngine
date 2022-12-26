#include "UI_UpperMenu.h"
#include "../Game.h"
#include "../imgui/imgui.h"

bool UI_UpperMenu::ButtonCenteredOnLine(const char* label, float alignment)
{
	ImGuiStyle& style = ImGui::GetStyle();

	float size = ImGui::CalcTextSize(label).x + style.FramePadding.x * 2.0f;
	float avail = ImGui::GetContentRegionAvail().x;

	float off = (avail - size) * alignment;
	if (off > 0.0f)
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);

	return ImGui::Button(label);
}

void UI_UpperMenu::Draw_UI_UpperMenu()
{
	ImGuiWindowClass upperMenu;
	upperMenu.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_AutoHideTabBar;
	ImGui::SetNextWindowClass(&upperMenu);
	if (ImGui::Begin("##upperMenu", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse))
	{
		bool flag = false;
		if (ButtonCenteredOnLine("Play", 0.5f))
		{

		};
		ImGui::SameLine();
		if (ButtonCenteredOnLine("Pause", 0))
		{

		};
		ImGui::SameLine();
		if (ButtonCenteredOnLine("Stop", 0))
		{

		};
	}ImGui::End();
}