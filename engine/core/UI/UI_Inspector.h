#pragma once

#include "../SimpleMath.h"

class Game;
class UserInterface;

class UI_Inspector
{
public:
	void Draw_UI_Inspector();
	void Init(Game* game);
	void InitUI(UserInterface* ui);

	void DrawActorTransform();
	void DrawActorComponents();

	bool ButtonCenteredOnLine(const char* label, float alignment = 0.5f);

	void ShowVector3(Vector3* vec3, const std::string& title);

private:
	Game* _game;
	UserInterface* _ui;
	bool isLocal = false;
};

