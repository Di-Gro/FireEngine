#pragma once

class Game;
class Actor;
class UserInterface;

class UI_Hierarchy
{
public:
	void Draw_UI_Hierarchy();
	void Init(Game* game);
	void InitUI(UserInterface* ui);
	void VisitActor(Actor* actor);

private:
	Game* _game;
	UserInterface* _ui;
};