#pragma once

#include "../SimpleMath.h"
#include "../CSBridge.h"

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

	bool ShowVector3(Vector3* vec3, const std::string& title);
	void AddComponent();
	void SearchComponent(char* searchText);
	
	static char textBuffer[1024];

	float widthComponent = 0;
private:
	Game* _game;
	UserInterface* _ui;
	bool isLocal = false;
};

FUNC(UI_Inspector, ShowText, bool)(CppRef gameRef, const char* label, const char* buffer, int length, size_t* ptr);