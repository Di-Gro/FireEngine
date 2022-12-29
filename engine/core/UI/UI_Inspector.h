#pragma once

#include "../SimpleMath.h"
#include "../imgui/imgui.h"
#include "../CSBridge.h"

class Game;
class UserInterface;

class UI_Inspector
{
public:
	void Draw_UI_Inspector();
	void Init(Game* game, UserInterface* ui);

	void DrawActorTransform();
	void DrawActorComponents();

	bool ButtonCenteredOnLine(const char* label, float alignment = 0.5f);

	bool ShowVector3(Vector3* vec3, const std::string& title);
	void AddComponent();
	void SearchComponent(char* searchText);

	static char textBuffer[1024];

	void BigSpace();
	void Space();
	float widthComponent = 0;
	CsRef csRef;

private:
	ImGuiTreeNodeFlags treeNodeFlags = 0
		| ImGuiTreeNodeFlags_DefaultOpen
		| ImGuiTreeNodeFlags_OpenOnArrow
		| ImGuiTreeNodeFlags_OpenOnDoubleClick
		| ImGuiTreeNodeFlags_Framed
		//| ImGuiTreeNodeFlags_SpanFullWidth
		;

	Game* _game;
	UserInterface* _ui;
	bool isLocal = false;

	ImVec2 m_compSpacing = { 0, 8 };
	ImVec2 m_lineSpacing = { 0, 3 };

private:
	void DrawComponent();
	void m_DrawHeader();
	void m_DrawItem(void(UI_Inspector::*func)());
};

FUNC(UI_Inspector, ShowText, bool)(CppRef gameRef, const char* label, const char* buffer, int length, size_t* ptr);
