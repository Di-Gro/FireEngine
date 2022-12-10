#include "Game.h"

#include <iostream>
#include <sstream>
#include <chrono>

#include "imgui\imgui.h"
#include "imgui\imgui_impl_dx11.h"
#include "imgui\imgui_impl_win32.h"

#include "Window.h"
#include "RenderTarget.h"
#include "FPSCounter.h"
#include "GameController.h"
#include "DirectionLight.h"
#include "FlyingCamera.h"
#include "ImageComponent.h"
#include "MeshComponent.h"
#include "Player.h"
#include "PlayerCamera.h"
#include "AmbientLight.h"

#include "MonoInst.h"
#include "ClassInfo.h"
#include "Refs.h"
#include "CppClass.h"
#include "CSComponent.h"


std::vector<std::string> game_shaderPaths = {
	Assets::ShaderVertexColor,
	Assets::ShaderDiffuseColor,
	Assets::ShaderDefault,
	Assets::ShaderDirectionLight,
	Assets::ShaderAmbientLight,
	"../../data/engine/shaders/rp_image.hlsl",
	"../../data/engine/shaders/shadow_map.hlsl",
	"../../data/engine/shaders/rp_screen_quad.hlsl",
	"../../data/engine/shaders/screen.hlsl",
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

	for (auto& path : game_shaderPaths)
		m_shaderAsset.CompileShader(path);

	m_lighting.Init(this);

	m_InitImGui();
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

	//csLink.Link(cppObj, "EngineMono", "CSClass");	

	//auto actor = CreateActor();
	//actor->AddComponent<MeshComponent>();
	//actor->AddComponent<CsComponent>("Engine.OhMyMesh");


	///

	m_defaultCamera = CreateActor("default camera")->AddComponent<FlyingCamera>();
	m_defaultCamera->localPosition({ 0, 0, 300 });
	m_defaultCamera->localRotation({ rad(-45), rad(45 + 180), 0 });
	m_defaultCamera->Attach();
	m_defaultCamera->drawDebug = false;

	m_lighting.f_directionLight = m_defaultCamera->AddComponent<DirectionLight>();
	m_lighting.f_directionLight->localRotation({ rad(-45), rad(45 + 180), 0 });
	m_lighting.f_directionLight->drawDebug(true);

	//auto ambientLight = m_defaultCamera->AddComponent<AmbientLight>();

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

	/// ImGui Update
	m_BeginUpdateImGui();

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

	if (m_hotkeys.GetButtonDown(Keys::Tilda))
		inFocus = !inFocus;
	
	if (!inFocus) {
		ImGui::Begin("Main Render Target");
		ImGui::Image(render()->screenSRV(), { 1920 / 6, 1061 / 6 });
		ImGui::End();

		ImGui::Begin("Direction Light Shadow Map");
		ImGui::Image(lighting()->directionLight()->depthResource()->get(), { 1920 / 6, 1061 / 6 });
		ImGui::End();
	}

	m_EndUpdateImGui();
}

void Game::m_Destroy() {

	auto it = m_actors.begin();
	while (it != m_actors.end()) {
		DestroyActor(*it);
		it = m_EraseActor(it);
	}

	m_DestroyImGui();
	m_hotkeys.Destroy();
	m_meshAsset.Destroy();
	m_render.Destroy();
	m_window.Destroy();
}

Actor* Game::CreateActor(Actor* parent, std::string name) {
	//std::cout << "+: Game.CreateActor()" << std::endl;

	auto cppRef = mono_create();
	
	Actor* actor = CppRefs::ThrowPointer<Actor>(cppRef);
	if (name != "")
		actor->SetName(name);

	if (parent != nullptr) {
		actor->parent(parent);
		actor->localScale({ 1, 1, 1 });
	}
	return actor;
}


GameObjectInfo Game::m_CreateActorFromCs(CsRef csRef, CppRef parentRef) {
	//std::cout << "+: Game.CreateGameObjectFromCS(): csRef:" << csRef << std::endl;

	auto classInfoRef = CppRefs::Create(ClassInfo::Get<Actor>());

	Actor* actor = new Actor();
	m_actors.insert(m_actors.end(), actor);

	actor->f_actorID = ++m_objectCount;
	actor->f_ref = CppRefs::Create(actor);
	actor->f_cppRef = actor->f_ref.cppRef();
	actor->f_csRef = csRef;

	actor->f_Init(this);

	if (parentRef.value != 0) {
		auto parent = CppRefs::ThrowPointer<Actor>(parentRef);
		actor->parent(parent);
		actor->localScale({ 1, 1, 1 });
	}
	GameObjectInfo objectInfo;
	objectInfo.classRef = RefCpp(classInfoRef.cppRef());
	objectInfo.objectRef = actor->cppRef();

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
	/*auto it = m_actors.begin();
	for (int i = 0; it != m_actors.end(); it++, i++) {
		if (!(*it)->HasParent())
			m_PrintSceneTree("", *it);
	}*/
}

void Game::m_PrintSceneTree(const std::string& prefix, const Actor* node) {
	/*if (node != nullptr) {
		std::cout << prefix << "* " << node->name;
		std::cout << " (" << node->f_actorID << ")" << std::endl;

		for (int i = 0; i < node->m_childs.size(); i++) {
			auto n = node->m_childs[i];
			m_PrintSceneTree(prefix + "  ", n);
		}
	}*/
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

/// ImGui 
/// -> 

void Game::m_InitImGui() {
	// Application init: create a dear imgui context, setup some options, load fonts
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	// TODO: Set optional io.ConfigFlags values, e.g. 'io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard' to enable keyboard controls.
	// TODO: Fill optional fields of the io structure later.
	// TODO: Load TTF/OTF fonts if you don't want to use the default font.

	io.ConfigFlags |= ImGuiConfigFlags_::ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_::ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_::ImGuiConfigFlags_ViewportsEnable;

	io.WantCaptureMouse = true;
			
	// Initialize helper Platform and Renderer backends (here we are using imgui_impl_win32.cpp and imgui_impl_dx11.cpp)
	ImGui_ImplWin32_Init(window()->GetHWindow());
	ImGui_ImplDX11_Init(render()->device(), render()->context());
}

void Game::m_DestroyImGui() {
	// Shutdown
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void Game::m_BeginUpdateImGui() {
	// Feed inputs to dear imgui, start new frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void Game::m_EndUpdateImGui() {
	ImGui::EndFrame();
	ImGui::Render();
	ImGui::UpdatePlatformWindows();
}

/// <-
/// ImGui 

int Game::GetRootActorsCount() {
	int count = 0;
	for (auto actor : m_actors) {
		if (!actor->HasParent())
			count++;
	}
	return count;
}

void Game::WriteRootActorsRefs(CsRef* refs) {
	CsRef* ptr = refs;
	for (auto actor : m_actors) {
		if (!actor->HasParent()) {
			*ptr = actor->csRef();
			ptr++;
		}
	}
}

DEF_FUNC(Game, CreateGameObjectFromCS, GameObjectInfo)(CppRef gameRef, CsRef csRef, CppRef parentRef) {
	return CppRefs::ThrowPointer<Game>(gameRef)->m_CreateActorFromCs(csRef, parentRef);
}

DEF_FUNC(Game, GetRootActorsCount, int)(CppRef gameRef) {
	return CppRefs::ThrowPointer<Game>(gameRef)->GetRootActorsCount();
}

DEF_FUNC(Game, WriteRootActorsRefs, void)(CppRef gameRef, CsRef* refs) {
	CppRefs::ThrowPointer<Game>(gameRef)->WriteRootActorsRefs(refs);
}