#pragma once

#include <iostream>
#include <chrono>
#include <list>
#include <cassert>
#include <string>

#include "Window.h"
#include "RenderDevice.h"
#include "Render.h"
#include "Lighting.h"
#include "FPSCounter.h"
#include "InputDevice.h"
#include "HotKeys.h"
#include "MeshAsset.h"
#include "ImageAsset.h"
#include "ShaderAsset.h"
#include "Math.h"
#include "CSBridge.h"

#include "GameObject.h"

class CameraComponent;
class ShadowMapRender;
class MonoInst;

class Game {

private:
	MonoInst* m_mono;

	Window m_window;
	Render m_render;
	Lighting m_lighting;
	InputDevice m_input;
	FPSCounter m_fpsCounter;
	HotKeys m_hotkeys;

	ShaderAsset m_shaderAsset;
	MeshAsset m_meshAsset;
	ImageAsset m_imageAsset;

	CameraComponent* m_defaultCamera = nullptr;
	CameraComponent* m_mainCamera = nullptr;

	std::list<GameObject*> m_gameObjects;

	bool m_onExit = false;
	int m_objectCount = 0;

public:

	Game() {}

	void Init(MonoInst* imono);
	void Run();
	void Exit(int code);
	
	inline MonoInst* mono() { return m_mono; }

	inline Window* window() { return &m_window; }
	inline Render* render() { return &m_render; }
	inline Lighting* lighting() { return &m_lighting; }
	inline InputDevice* input() { return &m_input; }
	inline HotKeys* hotkeys() { return &m_hotkeys; }

	inline ShaderAsset* shaderAsset() { return &m_shaderAsset; }
	inline MeshAsset* meshAsset() { return &m_meshAsset; }
	ImageAsset* imageAsset() { return &m_imageAsset; }

	inline CameraComponent* mainCamera() { return m_mainCamera; }
	void mainCamera(CameraComponent* camera) { m_mainCamera = camera; }

	const float& deltaTime() { return m_fpsCounter.GetDeltaTime(); }

	GameObject* CreateGameObject(std::string name = "");
	GameObject* CreateGameObjectCs(std::string name = "");

	GameObjectInfo CreateGameObjectFromCS(CsRef csRef);

	void DestroyGameObject(GameObject* gameObject);

	void PrintSceneTree();

	void SendGameMessage(const std::string& msg);

	std::list<GameObject*>::iterator BeginGameObject() { return m_gameObjects.begin(); }
	std::list<GameObject*>::iterator EndGameObject() { return m_gameObjects.end(); }
	
private:
	void m_InitMono(MonoInst* imono);

	void m_Update();
	void m_Destroy();

	std::list<GameObject*>::iterator m_EraseGameObject(std::list<GameObject*>::iterator it);

	void m_PrintSceneTree(const std::string& prefix, const GameObject* node);

};



FUNC(Game, CreateGameObjectFromCS, GameObjectInfo)(CppRef gameRef, CsRef csRef);

//FUNC(Game, AddComponentFromCS, CppObjectInfo)(CppRef gameRef, CsRef csRef);