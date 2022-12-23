#pragma once

class Game;

class UI_BarMenu
{
public:
	void Draw_UI_BarMenu();
	void Init(Game* game);

private:
	Game* _game;
};