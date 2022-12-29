#include "Game.h"

#include <iostream>
#include <sstream>
#include <chrono>

#include "imgui\imgui.h"
#include "imgui\imgui_impl_dx11.h"
#include "imgui\imgui_impl_win32.h"
#include "imguizmo\ImGuizmo.h"

#include "Window.h"
#include "Render.h"
#include "RenderDevice.h"
#include "Lighting.h"
#include "InputDevice.h"
#include "HotKeys.h"
#include "Assets.h"
#include "ShaderAsset.h"
#include "MeshAsset.h"
#include "ImageAsset.h"
#include "UI/UserInterface.h"
#include "Actor.h"

#include "RenderTarget.h"
#include "FPSCounter.h"
#include "GameController.h"
#include "DirectionLight.h"
#include "FlyingCamera.h"
#include "EditorCamera.h"
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
#include "RenderPassUI.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "TestLightComponent.h"


std::vector<std::string> game_shaderPaths = {
	Assets::ShaderVertexColor,
	Assets::ShaderDiffuseColor,
	Assets::ShaderDefault,
	Assets::ShaderDirectionLight,
	Assets::ShaderAmbientLight,
	Assets::ShaderPointLight,
	Assets::ShaderPointLightMesh,
	Assets::ShaderSpotLight,
	Assets::ShaderEditorHihglight,
	Assets::ShaderEditorOutline,
	Assets::ShaderBlur,
	"../../data/engine/shaders/rp_image.hlsl",
	"../../data/engine/shaders/shadow_map.hlsl",
	"../../data/engine/shaders/rp_screen_quad.hlsl",
	"../../data/engine/shaders/screen.hlsl",
};

Game::Game() {
	m_window = new Window();
	m_render = new Render();
	m_lighting = new Lighting();
	m_input = new InputDevice();
	m_hotkeys = new HotKeys();
	m_shaderAsset = new ShaderAsset();
	m_meshAsset = new MeshAsset();
	m_imageAsset = new ImageAsset();
	m_assets = new Assets();
	m_ui = new UserInterface();
}

Game::~Game() {
	delete m_window;
	delete m_render;
	delete m_lighting;
	delete m_input;
	delete m_hotkeys;
	delete m_shaderAsset;
	delete m_meshAsset;
	delete m_imageAsset;
	delete m_assets;
	delete m_ui;
}

void Game::Init(MonoInst* imono) {
	m_InitMono(imono);
		
	m_window->Init(L"CGLab6 - Shadow Map", 1920, 1080);
	m_window->Create();

	m_render->Init(this, m_window);
	m_shaderAsset->Init(m_render);
	m_meshAsset->Init(this);
	m_imageAsset->Init();
	m_input->Init(this);
	m_hotkeys->Init(this);
	m_assets->Init(this);

	for (auto& path : game_shaderPaths)
		m_shaderAsset->CompileShader(path);

	m_lighting->Init(this);

	m_InitImGui();
	m_ui->Init(this);
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
	bool isExitRequested = false;

	m_render->Start();
	m_meshAsset->Start(); 

	///

	auto cppObj = CppClass();
	auto csLink = CSLinked<CppClass>(mono());

	csLink.Link(cppObj, "EngineMono", "CSClass");	

	m_assets->CreateAssetId();
	//isExitRequested = true; // return;
	///
	m_editorCamera = CreateActor("editor camera")->AddComponent<EditorCamera>();
	m_editorCamera->localPosition({ 350, 403, -20 });
	m_editorCamera->localRotation({ -0.803, 1.781, 0 });
	
	m_defaultCamera = CreateActor("default camera")->AddComponent<FlyingCamera>();
	m_defaultCamera->localPosition({ 0, 0, 300 });
	m_defaultCamera->localRotation({ rad(-45), rad(45 + 180), 0 });
	m_defaultCamera->Attach();
	m_defaultCamera->drawDebug = false;

	m_lighting->f_directionLight = m_defaultCamera->AddComponent<DirectionLight>();
	m_lighting->f_directionLight->localRotation({ rad(-45), rad(45 + 180), 0 });
	m_lighting->f_directionLight->drawDebug(true);

	CreateActor("GameController")->AddComponent<GameController>();
	CreateActor()->AddComponent<TestLightComponent>();
	
	inFocus = false;
	m_lastGameCamera = mainCamera();
	m_editorCamera->Attach();

	MSG msg = {};
	
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
		m_render->Draw();
		
		m_fpsCounter.Update();
		if (m_fpsCounter.HasChanges()) {
			WCHAR text[256];
			swprintf_s(text, TEXT("FPS: %d"), m_fpsCounter.FPS());
			SetWindowText(m_window->GetHWindow(), text);
		}
	}

	m_Destroy();
}

void Game::Exit(int code) {
	if (!m_onExit) {
		m_onExit = true;
		m_window->Exit(code);
	}
}

void Game::m_Update() {
	/// Pre Update
	GameUpdateData updateData;
	updateData.deltaTime = deltaTime();
	mono_setUpdateData(updateData);
	m_hotkeys->Update(input());

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

	m_ui->Draw();
	/// Post Update
	m_hotkeys->LateUpdate();

	if (m_hotkeys->GetButtonDown(Keys::Tilda)) {
		inFocus = !inFocus;
		if (!inFocus) {
			m_lastGameCamera = mainCamera();
			m_editorCamera->Attach();
		}
		else {
			ui()->SelectedActor(nullptr);
			m_lastGameCamera->Attach();
			m_lastGameCamera = nullptr;
		}
	}

	if (!inFocus) {
		auto w = (float)window()->GetWidth();
		auto h = (float)window()->GetHeight();

		/*ImGui::Begin("Direction Light Shadow Map");
		ImGui::Image(lighting()->directionLight()->depthResource()->get(), { w / 6, h / 6 });
		ImGui::End();*/
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
	m_hotkeys->Destroy();
	m_meshAsset->Destroy();
	m_render->Destroy();
	m_window->Destroy();
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
		m_shaderAsset->RecompileShaders();
		meshAsset()->ReloadMaterials();
		std::cout << std::endl;
		return;
	}

	unsigned int objId;
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

#pragma region ImGuiStyle
	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
	style.Colors[ImGuiCol_ChildBg] = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.11f, 0.64f, 0.92f, 1.00f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.11f, 0.64f, 0.92f, 1.00f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.08f, 0.50f, 0.72f, 1.00f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);
	style.Colors[ImGuiCol_Separator] = style.Colors[ImGuiCol_Border];
	style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.41f, 0.42f, 0.44f, 1.00f);
	style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.29f, 0.30f, 0.31f, 0.67f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	style.Colors[ImGuiCol_Tab] = ImVec4(0.08f, 0.08f, 0.09f, 0.83f);
	style.Colors[ImGuiCol_TabHovered] = ImVec4(0.33f, 0.34f, 0.36f, 0.83f);
	style.Colors[ImGuiCol_TabActive] = ImVec4(0.23f, 0.23f, 0.24f, 1.00f);
	style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
	style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
	style.Colors[ImGuiCol_DockingPreview] = ImVec4(0.26f, 0.59f, 0.98f, 0.70f);
	style.Colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
	style.Colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.11f, 0.64f, 0.92f, 1.00f);
	style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
	style.GrabRounding = style.FrameRounding = 2.3f;
#pragma endregion ImGuiStyle
			
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
	ImGuizmo::BeginFrame();
}

void Game::m_EndUpdateImGui() {
	ImGui::EndFrame();
	ImGui::Render();
	ImGui::UpdatePlatformWindows();
}

/// <-
/// ImGui 

std::list<Actor*>::iterator Game::GetNextRootActors(const std::list<Actor*>::iterator& iter)
{
	for (auto it = iter; it != m_actors.end(); ++it)
	{
		if (!(*it)->HasParent())
			return it;
	}
	return m_actors.end();
}

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

//std::list<Actor*>::iterator Game::GetNextRootActor(const std::list<Actor*>::iterator& iter) {
//	for (auto it = iter; it != m_actors.end(); it++) {
//		if ((*it)->HasParent())
//			return it;
//	}
//	return m_actors.end();
//}

DEF_FUNC(Game, CreateGameObjectFromCS, GameObjectInfo)(CppRef gameRef, CsRef csRef, CppRef parentRef) {
	return CppRefs::ThrowPointer<Game>(gameRef)->m_CreateActorFromCs(csRef, parentRef);
}

DEF_FUNC(Game, GetRootActorsCount, int)(CppRef gameRef) {
	return CppRefs::ThrowPointer<Game>(gameRef)->GetRootActorsCount();
}

DEF_FUNC(Game, WriteRootActorsRefs, void)(CppRef gameRef, CsRef* refs) {
	CppRefs::ThrowPointer<Game>(gameRef)->WriteRootActorsRefs(refs);
}