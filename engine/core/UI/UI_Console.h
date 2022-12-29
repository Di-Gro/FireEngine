#pragma once

class Game;

class UI_Console
{
public:
	void Draw_UI_Console();
	void DrawConsoleButtons();
	void DrawConsoleInformation();

private:
	Game* _game;
};