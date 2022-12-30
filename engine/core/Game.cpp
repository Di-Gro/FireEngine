#include "Game.h"

#include <iostream>
#include <sstream>
#include <chrono>

#include "imgui\imgui.h"
#include "imgui\imgui_impl_dx11.h"
#include "imgui\imgui_impl_win32.h"
#include "imguizmo\ImGuizmo.h"

#include "Window.h"
#include "Scene.h"
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
	delete m_mainScene;
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

	m_mainScene = new Scene();
	CppRefs::Create(m_mainScene);
	m_mainScene->Init(this);
}

void Game::m_InitMono(MonoInst* imono) {
	m_mono = imono;

	auto gameType = m_mono->GetType("Engine", "Game");
	auto method_Init = mono::make_method_invoker<void(CppRef)>(gameType, "cpp_Init");

	auto gameRef = CppRefs::Create(this);
	method_Init(CppRef::Create(gameRef.cppRef()));
}

void Game::Run() {
	bool isExitRequested = false;

	PushScene(m_mainScene);

	m_render->Start();
	m_meshAsset->Start();

	///

	auto cppObj = CppClass();
	auto csLink = CSLinked<CppClass>(mono());

	csLink.Link(cppObj, "EngineMono", "CSClass");

	m_assets->CreateAssetId();
	//isExitRequested = true; // return;
	///
	m_editorCamera = scene()->CreateActor("editor camera")->AddComponent<EditorCamera>();
	m_editorCamera->localPosition({ 350, 403, -20 });
	m_editorCamera->localRotation({ -0.803, 1.781, 0 });

	m_defaultCamera = scene()->CreateActor("default camera")->AddComponent<FlyingCamera>();
	m_defaultCamera->localPosition({ 0, 0, 300 });
	m_defaultCamera->localRotation({ rad(-45), rad(45 + 180), 0 });
	m_defaultCamera->Attach();
	m_defaultCamera->drawDebug = false;

	m_lighting->f_directionLight = m_defaultCamera->AddComponent<DirectionLight>();
	m_lighting->f_directionLight->localRotation({ rad(-45), rad(45 + 180), 0 });
	m_lighting->f_directionLight->drawDebug(true);

	scene()->CreateActor("GameController")->AddComponent<GameController>();
	scene()->CreateActor()->AddComponent<TestLightComponent>();

	PopScene();

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
	m_callbacks.setUpdateData(updateData);
	m_hotkeys->Update(input());

	m_BeginUpdateImGui();

	/// Update
	m_mainScene->f_Update();

	ui()->selectedScene(m_mainScene);
	ui()->Draw();

	/// Post Update
	m_EndUpdateImGui();

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
}

void Game::m_Destroy() {

	m_mainScene->f_Destroy();

	m_DestroyImGui();
	m_hotkeys->Destroy();
	m_meshAsset->Destroy();
	m_render->Destroy();
	m_window->Destroy();
}

void Game::PushScene(Scene* value) {
	m_sceneStack.push_back(value);
	m_callbacks.setSceneRef(CppRefs::GetRef(value));
}

void Game::PopScene() {
	m_sceneStack.pop_back();
	m_callbacks.setSceneRef(RefCpp(0));
}

void Game::m_InitImGui() {
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	io.ConfigFlags |= ImGuiConfigFlags_::ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_::ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_::ImGuiConfigFlags_ViewportsEnable;

	io.WantCaptureMouse = true;
	io.WantCaptureKeyboard = true;

	io.KeysDown[io.KeyMap[ImGuiKey_Backspace]];

	ImGui_ImplWin32_Init(window()->GetHWindow());
	ImGui_ImplDX11_Init(render()->device(), render()->context());
}

void Game::m_DestroyImGui() {
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void Game::m_BeginUpdateImGui() {
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

void Game::Stat() {
	int actorsCount = 0;
	int csComponents = 0;
	int cppComponents = 0;

	for (auto it = m_mainScene->BeginActor(); it != m_mainScene->EndActor(); it++) {
		auto actor = *it;
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

DEF_FUNC(Game, SetGameCallbacks, void)(CppRef gameRef, const GameCallbacks& callbacks) {
	return CppRefs::ThrowPointer<Game>(gameRef)->callbacks(callbacks);
}
