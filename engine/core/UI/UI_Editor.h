#pragma once
#include "../imgui/imgui.h"

class Game;

class UI_Editor
{
public:
	void Draw_UI_Editor();
	void Draw_Tools();
	void Init(Game* game);
	bool ButtonCenteredOnLine(const char* label, float alignment = 0.5f);

private:
	Game* _game;
	const ImGuiViewport* viewport;
};