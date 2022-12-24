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
	bool hasClickInViewport = false;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	if (ImGui::Begin("Scene"))
	{
		auto viewportSize = (Vector2&)ImGui::GetWindowSize() - Vector2(0, 19.0f);
		auto mousePos = (Vector2&)ImGui::GetMousePos();
		mousePos -= (Vector2&)ImGui::GetWindowPos() + Vector2(0, 19.0f);
		mousePos = mousePos / viewportSize;

		_game->ui()->f_mouseViewportPosition = mousePos;
		
		Draw_Tools();
		_game->render()->ResizeViewport(viewportSize);
		ImGui::Image(_game->render()->screenSRV(), (ImVec2&)viewportSize);

		auto hasClick = ImGui::IsMouseClicked(ImGuiMouseButton_::ImGuiMouseButton_Left);
		hasClickInViewport = hasClick && ImGui::IsWindowHovered();
	}
	ImGui::End();
	ImGui::PopStyleVar();

	if (hasClickInViewport) {
		auto vpos = _game->ui()->mouseViewportPosition();
		auto actorRef = _game->render()->GetActorIdInViewport(vpos);
		if (actorRef != 0) {
			auto actor = CppRefs::ThrowPointer<Actor>(RefCpp(actorRef));
			_game->ui()->SelectedActor(actor);
		}
		else {
			_game->ui()->SelectedActor(nullptr);
		}
	}
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