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
class MeshAsset;
class ImageAsset;
class Assets;
class UserInterface;


extern std::vector<std::string> game_shaderPaths;

FUNC(Game, SetGameCallbacks, void)(CppRef gameRef, const GameCallbacks& callbacks);

class Game {
public:
	bool inFocus = true;

private:
	MonoInst* m_mono;

	Window* m_window;
	Render* m_render;
	Lighting* m_lighting;
	InputDevice* m_input;
	FPSCounter m_fpsCounter;
	HotKeys* m_hotkeys;

	ShaderAsset* m_shaderAsset;
	MeshAsset* m_meshAsset;
	ImageAsset* m_imageAsset;
	Assets* m_assets;

	UserInterface* m_ui;

	CameraComponent* m_defaultCamera = nullptr;
	CameraComponent* m_mainCamera = nullptr;
	CameraComponent* m_editorCamera = nullptr;
	CameraComponent* m_lastGameCamera = nullptr;

	Scene* m_mainScene;
	std::list<Scene*> m_sceneStack;

	GameCallbacks m_callbacks;

	bool m_onExit = false;

public:

	Game();
	~Game();

	void Init(MonoInst* imono);
	void Run();
	void Exit(int code);
	
	inline MonoInst* mono() { return m_mono; }

	inline Window* window() { return m_window; }
	inline Render* render() { return m_render; }
	inline Lighting* lighting() { return m_lighting; }
	inline InputDevice* input() { return m_input; }
	inline HotKeys* hotkeys() { return m_hotkeys; }
	inline UserInterface* ui() { return m_ui; }
	inline Assets* assets() { return m_assets; }
	inline Scene* scene() { return m_sceneStack.empty() ? nullptr : m_sceneStack.back(); }

	inline ShaderAsset* shaderAsset() { return m_shaderAsset; }
	inline MeshAsset* meshAsset() { return m_meshAsset; }
	ImageAsset* imageAsset() { return m_imageAsset; }

	inline CameraComponent* mainCamera() { return m_mainCamera; }
	void mainCamera(CameraComponent* camera) { m_mainCamera = camera; }

	const float& deltaTime() { return m_fpsCounter.GetDeltaTime(); }

	void Stat();

	GameCallbacks callbacks() { return m_callbacks; }
	void callbacks(const GameCallbacks& _callbacks) { m_callbacks = _callbacks; }
	
	void PushScene(Scene* value);
	void PopScene();
	
private:
	void m_InitMono(MonoInst* imono);
	void m_InitImGui();
	void m_DestroyImGui();

	void m_Update();
	void m_Destroy();

	void m_BeginUpdateImGui();
	void m_EndUpdateImGui();

};
