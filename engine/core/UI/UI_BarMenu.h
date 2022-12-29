#pragma once
#include "../Window.h"
class Game;

class UI_BarMenu
{
public:
	bool ButtonCenteredOnLine(const char* label, float alignment = 0.5f);
	void Draw_UI_BarMenu();
	void Init(Game* game);

	std::string OpenFileDialog(const char* filter);
	std::string SaveAsFileDialog(const char* filter);

private:
	Game* _game;
	Window win;
};