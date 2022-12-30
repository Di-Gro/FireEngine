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
#include "../SimpleMath.h"

class Game;

class UserInterface {
	friend class UI_Editor;

public:
	struct Callbacks
	{
		void (*onDrawComponent)(CsRef, float);
	};

	void Draw();
	void Init(Game* game);

	Scene* selectedScene() { return m_selectedScene; }
	void selectedScene(Scene* scene) { m_selectedScene = scene; }

	void SelectedActor(Actor* actor);
	void SetActorActive();

	inline Actor* GetActor() const { return _actor; }

	bool HasActor();
	bool isActive();

	Vector2 mouseViewportPosition() { return f_mouseViewportPosition; };
	Vector2 viewportPosition() { return f_viewportPosition; };

	void SetCallbacks(const Callbacks&);
	Callbacks _callbacks;

private: /// For class friends
	Vector2 f_mouseViewportPosition;
	Vector2 f_viewportPosition;

private:
	void InitMono();
	void InitDockSpace();
	void m_InitStyles();

	Game* _game;
	Actor* _actor;
	Scene* m_selectedScene = nullptr;

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
	UI_Inspector uiInspector;
};

FUNC(UserInterface, SetCallbacks, void)(CppRef cppRef, void(*func)(CsRef, float));
