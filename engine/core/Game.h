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

extern "C" __declspec(dllexport) GameObjectInfo Game_CreateGameObjectFromCS(CppRef gameRef, CsRef csRef, CppRef parentRef);

extern std::vector<std::string> game_shaderPaths;

class Game {
	friend GameObjectInfo Game_CreateGameObjectFromCS(CppRef gameRef, CsRef csRef, CppRef parentRef);

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

	std::list<Actor*> m_actors;

	bool m_onExit = false;
	unsigned int m_objectCount = 0;

	mono::mono_method_invoker<CppRef()> mono_create;
	mono::mono_method_invoker<void(GameUpdateData)> mono_setUpdateData;

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

	inline ShaderAsset* shaderAsset() { return m_shaderAsset; }
	inline MeshAsset* meshAsset() { return m_meshAsset; }
	ImageAsset* imageAsset() { return m_imageAsset; }

	inline CameraComponent* mainCamera() { return m_mainCamera; }
	void mainCamera(CameraComponent* camera) { m_mainCamera = camera; }

	const float& deltaTime() { return m_fpsCounter.GetDeltaTime(); }

	Actor* CreateActor(std::string name = "") { return CreateActor(nullptr, name); }
	Actor* CreateActor(Actor* parent, std::string name = "");

	void DestroyActor(Actor* actor);

	int GetRootActorsCount();
	void WriteRootActorsRefs(CsRef* refs);

	void PrintSceneTree();

	void SendGameMessage(const std::string& msg);

	void Stat();

	std::list<Actor*>::iterator GetNextRootActors(const std::list<Actor*>::iterator& iter);
	std::list<Actor*>::iterator BeginActor() { return m_actors.begin(); }
	std::list<Actor*>::iterator EndActor() { return m_actors.end(); }
	//std::list<Actor*>::iterator GetNextRootActor(const std::list<Actor*>::iterator& iter);
	
private:
	void m_InitMono(MonoInst* imono);
	void m_InitImGui();
	void m_DestroyImGui();

	void m_Update();
	void m_Destroy();

	void m_BeginUpdateImGui();
	void m_EndUpdateImGui();

	GameObjectInfo m_CreateActorFromCs(CsRef csRef, CppRef parentRef);

	std::list<Actor*>::iterator m_EraseActor(std::list<Actor*>::iterator it);

	void m_PrintSceneTree(const std::string& prefix, const Actor* node);

};

FUNC(Game, GetRootActorsCount, int)(CppRef gameRef);
FUNC(Game, WriteRootActorsRefs, void)(CppRef gameRef, CsRef* refs);
