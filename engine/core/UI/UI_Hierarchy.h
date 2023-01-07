#pragma once

class Game;
class Actor;
class UserInterface;

class UI_Hierarchy
{
public:
	void Draw_UI_Hierarchy();
	void Init(Game* game);
	void VisitActor(Actor* actor, int counter);

private:
	Game* _game;
	UserInterface* _ui;	

private:
	void m_DrawSceneContextMenu();
	void m_DrawActorContextMenu(Actor* actor);
	void m_PushPopupStyles();
	void m_PopPopupStyles();
};