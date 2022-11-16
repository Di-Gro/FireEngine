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

#include "GameObject.h"

class CameraComponent;
//class DirectionLight;
class ShadowMapRender;

class Game {

private:
	Window m_window;
	Render m_render;
	//RenderDevice m_renderTarget;
	Lighting m_lighting;
	InputDevice m_input;
	FPSCounter m_fpsCounter;
	HotKeys m_hotkeys;

	ShaderAsset m_shaderAsset;
	MeshAsset m_meshAsset;
	ImageAsset m_imageAsset;

	CameraComponent* m_defaultCamera = nullptr;
	CameraComponent* m_mainCamera = nullptr;
	//DirectionLight* f_directionLight = nullptr;

	std::list<GameObject*> m_gameObjects;

	bool m_onExit = false;
	int m_objectCount = 0;

public:

	Game() {}

	void Init();
	void Run();
	void Exit(int code);
	
	inline Window* window() { return &m_window; }
	inline Render* render() { return &m_render; }
	inline Lighting* lighting() { return &m_lighting; }
	inline InputDevice* input() { return &m_input; }
	inline HotKeys* hotkeys() { return &m_hotkeys; }

	inline ShaderAsset* shaderAsset() { return &m_shaderAsset; }
	inline MeshAsset* meshAsset() { return &m_meshAsset; }
	inline ImageAsset* imageAsset() { return &m_imageAsset; }

	inline CameraComponent* mainCamera() { return m_mainCamera; }
	void mainCamera(CameraComponent* camera) { m_mainCamera = camera; }

	//DirectionLight* directionLight() { return f_directionLight; }

	const float& deltaTime() { return m_fpsCounter.GetDeltaTime(); }

	GameObject* CreateGameObject(std::string name = "");
	void DestroyGameObject(GameObject* gameObject);

	void PrintSceneTree();

	void SendGameMessage(const std::string& msg);

	std::list<GameObject*>::iterator BeginGameObject() { return m_gameObjects.begin(); }
	std::list<GameObject*>::iterator EndGameObject() { return m_gameObjects.end(); }
	
private:

	void m_Update();
	//void m_Draw();
	void m_Destroy();

	std::list<GameObject*>::iterator m_EraseGameObject(std::list<GameObject*>::iterator it);

	void m_PrintSceneTree(const std::string& prefix, const GameObject* node);

};
