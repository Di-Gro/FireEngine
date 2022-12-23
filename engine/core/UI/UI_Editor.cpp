#include "UI_Editor.h"
#include "../Game.h"

bool UI_Editor::ButtonCenteredOnLine(const char* label, float alignment)
{
	ImGuiStyle& style = ImGui::GetStyle();

	float size = ImGui::CalcTextSize(label).x + style.FramePadding.x * 2.0f;
	float avail = ImGui::GetContentRegionAvail().x;

	float off = (avail - size) * alignment;
	if (off > 0.0f)
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);

	return ImGui::Button(label);
}

void UI_Editor::Draw_UI_Editor()
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	if (ImGui::Begin("Scene"))
	{
		Draw_Tools();
		auto size = ImGui::GetWindowSize();
		_game->render()->ResizeViewport(size.x, size.y - 20.0f);
		ImGui::Image(_game->render()->screenSRV(), { size.x, size.y - 20.0f });
	}ImGui::End();
	ImGui::PopStyleVar();
}

void UI_Editor::Draw_Tools()
{
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
	ImGuiWindowClass tools;
	tools.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_AutoHideTabBar;
	ImGui::SetNextWindowPos({ ImGui::GetWindowPos().x + 5.0f, ImGui::GetWindowPos().y + 25.0f });
	ImGui::SetNextWindowClass(&tools);
	if (ImGui::Begin("##tools", NULL, ImGuiWindowFlags_NoTitleBar))
	{
		const char* items[] = { "Local", "World" };
		static const char* current_item = items[0];

		ImGui::PushItemWidth(80.0f);
		if (ImGui::BeginCombo("##combo", current_item))
		{
			for (int n = 0; n < IM_ARRAYSIZE(items); n++)
			{
				bool is_selected = (current_item == items[n]);
				if (ImGui::Selectable(items[n], is_selected))
					current_item = items[n];
					if (is_selected)
						ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		ImGui::PopItemWidth();

	}ImGui::End();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
}

void UI_Editor::Init(Game* game)
{
	_game = game;
	viewport = ImGui::GetMainViewport();
}