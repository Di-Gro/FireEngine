#include "UI_ContentBrowser.h"
#include "../Game.h"
#include "../imgui/imgui.h"

void UI_ContentBrowser::Draw_UI_ContentBrowser()
{
	if (ImGui::Begin("Content Browser"))
	{
		
	}ImGui::End();
	
	Draw_InnerContentBrowser();
}

void UI_ContentBrowser::Draw_InnerContentBrowser()
{
	ImGuiWindowClass inner_content_browser;
	inner_content_browser.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_AutoHideTabBar;
	ImGui::SetNextWindowClass(&inner_content_browser);
	if (ImGui::Begin("##inner_content_browser", NULL, ImGuiWindowFlags_NoTitleBar))
	{

	}ImGui::End();
}