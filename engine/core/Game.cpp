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
#include "Player.h"
#include "PlayerCamera.h"

#include "MonoInst.h"
#include "ClassInfo.h"
#include "Refs.h"
#include "CppClass.h"
#include "CSComponent.h"


std::vector<std::string> shaderPaths = {
	 "../../data/engine/shaders/vertex_color.hlsl",
	 "../../data/engine/shaders/defuse_color.hlsl",
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
	mono_setUpdateData = mono::make_method_invoker<void(GameUpdateData)>(gameType, "cpp_SetUpdateData");

	auto gameObjectType = m_mono->GetType("Engine", "Actor");
	mono_create = mono::make_method_invoker<CppRef()>(gameObjectType, "cpp_Create");

	auto gameRef = CppRefs::Create(this);
	method_SetGameRef(CppRef::Create(gameRef.cppRef()));
}

void Game::Run() {
	m_render.Start();
	m_meshAsset.Start();

	///

	auto cppObj = CppClass();
	auto csLink = CSLinked<CppClass>(mono());

	csLink.Link(cppObj, "EngineMono", "CSClass");	

	///

	m_defaultCamera = CreateActor("default camera")->AddComponent<FlyingCamera>();
	m_defaultCamera->localPosition({ 0, 0, 300 });
	m_defaultCamera->localRotation({ rad(-45), rad(45 + 180), 0 });
	m_defaultCamera->Attach();
	m_defaultCamera->drawDebug = false;

	m_lighting.f_directionLight = m_defaultCamera->AddComponent<DirectionLight>();
	m_lighting.f_directionLight->localRotation({ rad(-45), rad(45 + 180), 0 });
	m_lighting.f_directionLight->drawDebug(true);

	CreateActor("GameController")->AddComponent<GameController>();
	

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
	/// Pre Update
	GameUpdateData updateData;
	updateData.deltaTime = deltaTime();
	mono_setUpdateData(updateData);
	m_hotkeys.Update(input());

	/// Update
	auto it = m_actors.begin();
	while (it != m_actors.end()) {

		auto actor = (*it);
		if (actor->IsDestroyed()) {
			actor->friend_timeToDestroy--;
			if (actor->friend_timeToDestroy <= 0)
				it = m_EraseActor(it);
			continue;
		}
		actor->f_Update();
		it++;
	}

	/// Post Update
	m_hotkeys.LateUpdate();
}

void Game::m_Destroy() {
	m_hotkeys.Destroy();
	m_window.Destroy();

	auto it = m_actors.begin();
	while (it != m_actors.end()) {
		DestroyActor(*it);
		it = m_EraseActor(it);
	}


}

Actor* Game::CreateActor(std::string name) {
	//std::cout << "+: Game.CreateActor()" << std::endl;

	auto cppRef = mono_create();
	
	Actor* actor = CppRefs::ThrowPointer<Actor>(cppRef);
	actor->name = name != "" ? name : "Actor";

	return actor;
}

GameObjectInfo Game::m_CreateActor(CsRef csRef, std::string name) {
	//std::cout << "+: Game.CreateGameObjectFromCS(): csRef:" << csRef << std::endl;

	auto classInfoRef = CppRefs::Create(ClassInfo::Get<Actor>());

	Actor* actor = new Actor();
	m_actors.insert(m_actors.end(), actor);

	actor->f_actorID = ++m_objectCount;
	actor->f_ref = CppRefs::Create(actor);
	actor->f_cppRef = actor->f_ref.cppRef();
	actor->f_csRef = csRef;

	actor->f_Init(this, name);
		
	GameObjectInfo objectInfo;
	objectInfo.classRef = RefCpp(classInfoRef.cppRef());
	objectInfo.objectRef = actor->cppRef();
	//objectInfo.transformRef = actor->transformCsRef();

	return objectInfo;
}

void Game::DestroyActor(Actor* actor) {
	if (actor->ActorBase::friend_CanDestroy()) {
		actor->ActorBase::friend_StartDestroy();

		actor->f_Destroy();

		if (CppRefs::IsValid(actor->f_ref)) {
			CppRefs::Remove(actor->f_ref);
			//std::cout << "+: CppRefs.Remove(): " << actor->cppRef() << std::endl;
		}
	}
}

std::list<Actor*>::iterator Game::m_EraseActor(std::list<Actor*>::iterator it) {
	auto* actor = *it;
	auto next = m_actors.erase(it);
	delete actor;
	return next;
}

void Game::PrintSceneTree() {
	auto it = m_actors.begin();
	for (int i = 0; it != m_actors.end(); it++, i++) {
		if (!(*it)->HasParent())
			m_PrintSceneTree("", *it);
	}
}

void Game::m_PrintSceneTree(const std::string& prefix, const Actor* node) {
	if (node != nullptr) {
		std::cout << prefix << "* " << node->name;
		std::cout << " (" << node->f_actorID << ")" << std::endl;

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

	for (auto actor : m_actors) {
		if (actor->f_actorID == objId && !actor->IsDestroyed())
			actor->RecieveGameMessage(s);
	}
}

void Game::Stat() {
	int actorsCount = 0;
	int csComponents = 0;
	int cppComponents = 0;

	for (auto* actor : m_actors) {
		actorsCount++;

		int count = actor->GetComponentsCount();
		size_t* refs = new size_t[count]();
		actor->WriteComponentsRefs(refs);

		for (int i = 0; i < count; i++) {
			if (refs[i] == 0)
				cppComponents++;
			else
				csComponents++;
		}

		delete refs;
	}

	std::cout << "+ Game.Stat() -> " << std::endl;
	std::cout << "+ actorsCount: " << actorsCount << std::endl;
	std::cout << "+ csComponents: " << csComponents << std::endl;
	std::cout << "+ cppComponents: " << cppComponents << std::endl;
	std::cout << "+ <- " << std::endl;
}

DEF_FUNC(Game, CreateGameObjectFromCS, GameObjectInfo)(CppRef gameRef, CsRef csRef, const char* name) {
	return CppRefs::ThrowPointer<Game>(gameRef)->m_CreateActor(csRef, name);
}