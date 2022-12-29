#pragma once

#include "UI_Hierarchy.h"
#include "UI_Inspector.h"
#include "UI_Editor.h"
#include "UI_BarMenu.h"
#include "UI_UpperMenu.h"
#include "UI_PlayGame.h"
#include "UI_ContentBrowser.h"
#include "UI_Console.h"
#include "../imgui/imgui.h"
#include "../Actor.h"

class Game;

class UserInterface
{
public:
	struct Callbacks
	{
		void (*onDrawComponent)(CsRef, float);
	};

	void Draw();
	void Init(Game* game);
	
	void SelectedActor(Actor* actor);
	void SetActorActive();
	
	inline Actor* GetActor() const { return _actor; }
	
	bool HasActor();
	bool isActive();

	void SetCallbacks(const Callbacks&);
	Callbacks _callbacks;
	UI_Inspector uiInspector;
private:

	void InitMono();
	void InitDockSpace();

	Game* _game;
	Actor* _actor;

	static ImGuiDockNodeFlags dockspace_flags;
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

	static bool opt_fullscreen;
	static bool dockspaceOpen;
	static bool opt_padding;
	bool _isActorActive = false;

	UI_Hierarchy uiHierarchy;
	UI_Editor uiEditor;
	UI_BarMenu uiBarMenu;
	UI_UpperMenu uiUpperMenu;
	UI_PlayGame uiPlayGame;
	UI_ContentBrowser uiContentBrowser;
	UI_Console uiConsole;
};

FUNC(UserInterface, SetCallbacks, void)(CppRef cppRef, void(*func)(CsRef, float));