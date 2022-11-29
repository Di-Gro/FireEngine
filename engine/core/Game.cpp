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

#include "MonoInst.h"
#include "ClassInfo.h"
#include "Refs.h"
#include "CppClass.h"
#include "CSComponent.h"


std::vector<std::string> shaderPaths = {
	 "../../data/engine/shaders/vertex_color.hlsl",
	 "../../data/engine/shaders/default.hlsl",
	 "../../data/engine/shaders/image.hlsl",
	 "../../data/engine/shaders/shadow_map.hlsl",
	 "../../data/engine/shaders/screen_quad.hlsl",
};


void Game::Init(MonoInst* imono) {
	m_InitMono(imono);
		
	m_window.Init(L"CGLab6 - Shadow Map", 1920, 1080);
	m_window.Create();

	m_render.Init(this, &m_window);
	m_shaderAsset.Init(&m_render);
	m_meshAsset.Init(this);
	m_imageAsset.Init();
	m_input.Init(this);
	m_hotkeys.Init(this);

	for (auto& path : shaderPaths)
		m_shaderAsset.CompileShader(path);

	m_lighting.Init(this);
}

void Game::m_InitMono(MonoInst* imono) {
	m_mono = imono;

	auto gameType = m_mono->GetType("Engine", "Game");
	auto method_SetGameRef = mono::make_method_invoker<void(CppRef)>(gameType, "cpp_SetGameRef");

	auto gameRef = Refs::Create(this);
	method_SetGameRef(CppRef::Create(gameRef.id()));
}

void Game::Run() {
	m_render.Start();
	m_meshAsset.Start();

	///
	
	std::string mesnName1 = "../../data/assets/levels/farm/meshes/House_Purple.obj";
	std::string mesnName2 = "../../data/assets/levels/farm/meshes/Coffin.obj";
	std::string mesnName3 = "../../data/assets/levels/farm/meshes/Daisy.obj";

	auto cppObj = CppClass();
	auto csLink = CSLinked<CppClass>(mono());

	csLink.Link(cppObj, "EngineMono", "CSClass");

	//auto gameObject = CreateGameObjectCs();

	///

	m_defaultCamera = CreateGameObject("default camera")->AddComponent<FlyingCamera>();
	m_defaultCamera->transform->localPosition({ 0, 0, 300 });
	m_defaultCamera->transform->localRotation({ rad(-45), rad(45 + 180), 0 });
	m_defaultCamera->Attach();
	m_defaultCamera->drawDebug = false;

	m_lighting.f_directionLight = m_defaultCamera->AddComponent<DirectionLight>();
	m_lighting.f_directionLight->transform->localRotation({ rad(-45), rad(45 + 180), 0 });
	m_lighting.f_directionLight->drawDebug(true);

	CreateGameObject("GameController")->AddComponent<GameController>();
	

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
		m_hotkeys.LateUpdate();

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
	m_hotkeys.Destroy();

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
	gameObject->f_objectID = ++m_objectCount;

	return gameObject;
}

GameObject* Game::CreateGameObjectCs(std::string name) {
	std::cout << "+: Game.CreateGameObjectCs()" << std::endl;

	auto gameType = m_mono->GetType("Engine", "GameObject");
	auto method_create = mono::make_method_invoker<CppRef()>(gameType, "cpp_Create");

	auto cppRef = method_create();

	return Refs::GetPointer<GameObject>(cppRef);
}

GameObjectInfo Game::CreateGameObjectFromCS(CsRef csRef) {
	std::cout << "+: Game.CreateGameObjectFromCS(): csRef:" << csRef << std::endl;

	auto classInfoRef = Refs::Create(ClassInfo::Get<GameObject>());

	GameObject* gameObject = CreateGameObject();
	GameObjectBase* ptr2 = (GameObjectBase*)gameObject;
	CsLink* ptr3= (CsLink*)gameObject;
	GameObject* ptr4 = (GameObject*)ptr3;

	gameObject->f_ref = Refs::Create(gameObject);
	gameObject->f_cppRef = gameObject->f_ref.id();
	gameObject->f_csRef = csRef;

	GameObjectInfo objectInfo;
	objectInfo.classRef = RefCpp(classInfoRef.id());
	objectInfo.objectRef = gameObject->cppRef();
	objectInfo.transformRef = gameObject->transform->csRef();

	return objectInfo;
}

void Game::DestroyGameObject(GameObject* gameObject) {
	if (gameObject->GameObjectBase::friend_CanDestroy()) {
		gameObject->GameObjectBase::friend_StartDestroy();

		gameObject->f_Destroy();

		if (Refs::IsValid(gameObject->f_ref)) {
			Refs::Remove(gameObject->f_ref);
			std::cout << "+: Refs.Remove(): " << gameObject->cppRef() << std::endl;
		}
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
		std::cout << " (" << node->f_objectID << ")" << std::endl;

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
		if (gameObject->f_objectID == objId && !gameObject->IsDestroyed())
			gameObject->RecieveGameMessage(s);
	}
}

DEF_FUNC(Game, CreateGameObjectFromCS, GameObjectInfo)(CppRef gameRef, CsRef csRef) {
	return Refs::ThrowPointer<Game>(gameRef)->CreateGameObjectFromCS(csRef);
}