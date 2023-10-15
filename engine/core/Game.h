#pragma once

#include <iostream>
#include <chrono>
#include <list>
#include <cassert>
#include <string>

#include "FPSCounter.h"
#include "Math.h"
#include "CSBridge.h"
#include "CSLinked.h"

#include "EditorSettings.h"

class Actor;
class Scene;
class CameraComponent;
class MonoInst;
class Window;
class Render;
class Lighting;
class InputDevice;
class HotKeys;
class ShaderAsset;
class Assets;
class UserInterface;
class AssetStore;
class MaterialResource;
class SceneWindow;
class Physics;
class NavMesh;

extern std::vector<std::string> game_shaderPaths;

FUNC(Game, SetGameCallbacks, void)(CppRef gameRef, const GameCallbacks& callbacks);

class Game {
public:
	bool inFocus = false;
	EditorSettings editorSettings;

private:
	MonoInst* m_mono;

	Window* m_window;
	Render* m_render;
	Physics* m_physics;
	//Lighting* m_lighting;
	InputDevice* m_input;
	FPSCounter m_updateTimer;
	FixedTimer m_fixedTimer;
	HotKeys* m_hotkeys;

	ShaderAsset* m_shaderAsset;
	Assets* m_assets;
	AssetStore* m_assetStore;

	NavMesh* m_NavMesh;

	UserInterface* m_ui;

	//CameraComponent* m_defaultCamera = nullptr;
	CameraComponent* m_mainCamera = nullptr;
	//CameraComponent* m_editorCamera = nullptr;
	//CameraComponent* m_lastGameCamera = nullptr;

	SceneWindow* m_editorWindow;
	SceneWindow* m_gameWindow;

	Scene* m_editorScene;
	Scene* m_gameScene = nullptr;
	Scene* m_nextScene = nullptr;

	std::list<Scene*> m_scenes;
	std::list<Scene*> m_sceneStack;

	GameCallbacks m_callbacks;

	bool m_onExit = false;
	//bool m_isEditor = false;

	std::string editorSceneAssetName;
	int editorSceneAssetIdHash;

public:

	Game();
	~Game();

	void Init(MonoInst* imono);
	void Run();
	void Exit(int code);

	void m_ShowFPS();

	inline MonoInst* mono() { return m_mono; }

	inline Window* window() { return m_window; }
	inline Render* render() { return m_render; }
	inline Physics* physics() { return m_physics; }
	//inline Lighting* lighting() { return m_lighting; }
	inline InputDevice* input() { return m_input; }
	inline HotKeys* hotkeys() { return m_hotkeys; }
	inline UserInterface* ui() { return m_ui; }
	inline Assets* assets() { return m_assets; }
	inline AssetStore* assetStore() { return m_assetStore; }
	inline Scene* currentScene() { return m_sceneStack.empty() ? nullptr : m_sceneStack.back(); }

	inline ShaderAsset* shaderAsset() { return m_shaderAsset; }

	inline NavMesh* navMesh() { return m_NavMesh; };

	//inline bool isEditor() { return m_isEditor; }

	inline CameraComponent* mainCamera() { return m_mainCamera; }
	void mainCamera(CameraComponent* camera) {  m_mainCamera = camera; };

	const float& deltaTime() { return m_updateTimer.GetDelta(); }
	const float& deltaFixedTime() { return m_fixedTimer.GetDelta(); }

	bool IsPlayMode() { return m_gameScene != nullptr; }

	//void Stat();

	const GameCallbacks& callbacks() { 
		return m_callbacks; 
	}

	void callbacks(const GameCallbacks& _callbacks) { 
		m_callbacks = _callbacks; 
		int i = 9;
	}

	void PushScene(Scene* value);
	void PopScene();

	Scene* CreateScene(bool isEditor);
	Scene* CreateScene(bool isEditor, const std::string& assetId);
	void DestroyScene(Scene* scene);

	bool CanChangeScene();
	void ChangeScene(Scene* scene);

	inline std::list<Scene*>::iterator ScenesBegin() { return m_scenes.begin(); }
	inline std::list<Scene*>::iterator ScenesEnd() { return m_scenes.end(); }

	void DeleteMaterialFromAllScenes(const MaterialResource* material);

	void TogglePlayMode();
	void ToggleGameFocus();

	//bool LoadScene(Scene* targetScene, const char* assetId = nullptr);
	bool LoadScene(Scene* targetScene, int assetGuidHash = 0);


private:
	void m_InitMono(MonoInst* imono);
	void m_InitImGui();
	void m_DestroyImGui();

	void m_BeginUpdate();
	void m_EndUpdate();
	void m_Destroy();
	void m_DrawUI();
	void m_ForScenes(void (Scene::* method)());

	std::list<Scene*>::iterator m_EraseScene(std::list<Scene*>::iterator iter);

	void m_BeginUpdateImGui();
	void m_EndUpdateImGui();

};

FUNC(Game, mainCamera_get, CsRef)(CppRef gameRef);
FUNC(Game, Exit, void)(CppRef gameRef);

FUNC(Game, PushScene, void)(CppRef gameRef, CppRef sceneRef);
FUNC(Game, PopScene, void)(CppRef gameRef);

FUNC(Game, CreateScene, CppRef)(CppRef gameRef, bool isEditor);
FUNC(Game, DestroyScene, void)(CppRef gameRef, CppRef sceneRef);
