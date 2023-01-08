#pragma once
#include <unordered_map>

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
class SceneWindow;

class UserInterface {
	friend class UI_Editor;

public:
	struct Callbacks {
		void (*onDrawComponent)(CsRef, float);
		void (*requestComponentName)(CsRef);
	};

private:
	static bool opt_fullscreen;
	static bool dockspaceOpen;
	static bool opt_padding;

public:
	Callbacks _callbacks;

	ShaderResource icMove;
	ShaderResource icRotate;
	ShaderResource icScale;
	ShaderResource icPickup;
	ShaderResource icPickupActor;
	ShaderResource icPickupComponent;
	ShaderResource icPickupAsset;

private: /// For friends
	//Vector2 f_mouseViewportPosition;
	//Vector2 f_viewportPosition;

private:
	Game* _game;
	Actor* _actor;
	Scene* m_selectedScene = nullptr;

	bool _isActorActive = false;

	UI_Hierarchy uiHierarchy;
	UI_Editor uiEditor;
	UI_BarMenu uiBarMenu;
	UI_UpperMenu uiUpperMenu;
	UI_PlayGame uiPlayGame;
	UI_ContentBrowser uiContentBrowser;
	UI_Console uiConsole;
	UI_Inspector uiInspector;

	static ImGuiDockNodeFlags dockspace_flags;
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

	std::unordered_map<size_t, SceneWindow*> m_sceneWindows;

	Image m_imgMove;
	Image m_imgRotate;
	Image m_imgScale;
	Image m_imgPickup;
	Image m_imgPickupActor;
	Image m_imgPickupComponent;
	Image m_imgPickupAsset;

	Texture m_texMove;
	Texture m_texRotate;
	Texture m_texScale;
	Texture m_texPickup;
	Texture m_texPickupActor;
	Texture m_texPickupComponent;
	Texture m_texPickupAsset;

public:
	UserInterface();
	~UserInterface();

	void Draw();
	void Init(Game* game);
	void Destroy();

	UI_Inspector* inspector() { return &uiInspector; }

	Scene* selectedScene() { return m_selectedScene; }
	void selectedScene(Scene* scene) { m_selectedScene = scene; }

	void SelectedActor(Actor* actor);
	void SetActorActive();

	inline Actor* GetActor() const { return _actor; }

	bool HasActor();
	bool isActive();

	//Vector2 mouseViewportPosition() { return f_mouseViewportPosition; };
	//Vector2 viewportPosition() { return f_viewportPosition; };

	void SetCallbacks(const Callbacks&);

	bool HasSceneWindow(const std::string& sceneId);
	SceneWindow* GetSceneWindow(const std::string& sceneId);
	SceneWindow* CreateSceneWindow(const std::string& sceneId, const std::string& name);
	void RemoveSceneWindow(const std::string& sceneId);

private:
	void InitMono();
	void InitDockSpace();
	void m_InitStyles();
	void m_InitIcons();

	size_t m_StringHash(const std::string& str) { return std::hash<std::string>()(str); }
};

FUNC(UserInterface, SetCallbacks2, void)(CppRef cppRef, const UserInterface::Callbacks& callbacks);
