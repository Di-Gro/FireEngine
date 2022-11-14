#include "Game.h"

#include <iostream>
#include <sstream>
#include <chrono>

#include "Window.h"
#include "RenderTarget.h"
#include "FPSCounter.h"
#include "GameController.h"
#include "DirectionLight.h"
#include "FlyingCamera.h"
#include "ShadowMapRender.h"
#include "ImageComponent.h"
#include "MeshComponent.h"


std::vector<std::string> shaderPaths = {
	 "data/engine/shaders/vertex_color.hlsl",
	 "data/engine/shaders/default.hlsl",
	 "data/engine/shaders/image.hlsl",
	 "data/engine/shaders/shadow_map.hlsl",
	 "data/engine/shaders/screen_quad.hlsl",
};


void Game::Init() {
	m_window.Init(L"CGLab6 - Shadow Map", 1920, 1080);
	m_window.Create();

	m_render.Init(this, &m_window);
	m_shaderAsset.Init(&m_render);
	m_meshAsset.Init(this);
	m_imageAsset.Init();
	m_input.Init(this);

	for (auto& path : shaderPaths)
		m_shaderAsset.CompileShader(path);

	m_lighting.Init(this);
}

void Game::Run() {

	m_render.Start();

	m_defaultCamera = CreateGameObject("default camera")->AddComponent<FlyingCamera>();
	m_defaultCamera->transform.localPosition({ 0, 0, 300 });
	m_defaultCamera->transform.localRotation({ rad(-45), rad(45 + 180), 0 });
	m_defaultCamera->Attach();
	m_defaultCamera->drawDebug = false;

	m_lighting.f_directionLight = m_defaultCamera->AddComponent<DirectionLight>();
	m_lighting.f_directionLight->transform.localRotation({ rad(-45), rad(45 + 180), 0 });
	m_lighting.f_directionLight->drawDebug(true);

	CreateGameObject("GameController")->AddComponent<GameController>();

	auto mesh = CreateGameObject()->AddComponent<MeshComponent>();

	auto form = Forms4::Box({ 50,50,50 });
	mesh->CreateMesh(&form.verteces, &form.indexes, "data/engine/shaders/shadow_map.hlsl");


	MSG msg = {};
	bool isExitRequested = false;
	while (!isExitRequested) {
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (msg.message == WM_QUIT || m_onExit) {
			isExitRequested = true;
			break;
		}

		m_hotkeys.Update(input());
		m_Update();
		m_render.Draw();

		m_fpsCounter.Update();
		if (m_fpsCounter.HasChanges()) {
			WCHAR text[256];
			swprintf_s(text, TEXT("FPS: %d"), m_fpsCounter.FPS());
			SetWindowText(m_window.GetHWindow(), text);
		}
	}

	m_Destroy();
}

void Game::Exit(int code) {
	if (!m_onExit) {
		m_onExit = true;
		m_window.Exit(code);
	}
}

void Game::m_Update() {
	auto it = m_gameObjects.begin();
	while (it != m_gameObjects.end()) {

		auto gameObject = (*it);
		if (gameObject->IsDestroyed()) {
			gameObject->friend_timeToDestroy--;
			if (gameObject->friend_timeToDestroy <= 0)
				it = m_EraseGameObject(it);
			continue;
		}
		gameObject->f_Update();
		it++;
	}
}

void Game::m_Destroy() {
	auto it = m_gameObjects.begin();
	while (it != m_gameObjects.end()) {
		DestroyGameObject(*it);
		it = m_EraseGameObject(it);
	}
}

GameObject* Game::CreateGameObject(std::string name) {
	GameObject* gameObject = new GameObject();
	auto it = m_gameObjects.insert(m_gameObjects.end(), gameObject);

	gameObject->f_Init(this, name);
	gameObject->friend_objectID = ++m_objectCount;

	return gameObject;
}

void Game::DestroyGameObject(GameObject* gameObject) {
	if (gameObject->GameObjectBase::friend_CanDestroy()) {
		gameObject->GameObjectBase::friend_StartDestroy();

		gameObject->f_Destroy();
	}
}

std::list<GameObject*>::iterator Game::m_EraseGameObject(std::list<GameObject*>::iterator it) {
	auto* gameObject = *it;
	auto next = m_gameObjects.erase(it);
	delete gameObject;
	return next;
}

void Game::PrintSceneTree() {
	auto it = m_gameObjects.begin();
	for (int i = 0; it != m_gameObjects.end(); it++, i++) {
		if (!(*it)->HasParent())
			m_PrintSceneTree("", *it);
	}
}

void Game::m_PrintSceneTree(const std::string& prefix, const GameObject* node) {
	if (node != nullptr) {
		std::cout << prefix << "* " << node->name;
		std::cout << " (" << node->friend_objectID << ")" << std::endl;

		for (int i = 0; i < node->m_childs.size(); i++) {
			auto n = node->m_childs[i];
			m_PrintSceneTree(prefix + "  ", n);
		}
	}
}

void Game::SendGameMessage(const std::string& msg) {

	if (msg == "scene") {
		PrintSceneTree();
		return;
	}

	if (msg == "recompile") {
		m_shaderAsset.RecompileShaders();
		meshAsset()->ReloadMaterials();
		std::cout << std::endl;
		return;
	}

	int objId;
	std::stringstream ss(msg);
	ss >> objId;

	std::string lostMsg;
	std::getline(ss, lostMsg);
	auto s = lostMsg.erase(0, 1);

	for (auto gameObject : m_gameObjects) {
		if (gameObject->friend_objectID == objId && !gameObject->IsDestroyed())
			gameObject->RecieveGameMessage(s);
	}
}
