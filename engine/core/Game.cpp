#include "Game.h"

#include <iostream>
#include <sstream>
#include <chrono>
#include "NavMesh.h"
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
#include "AssetStore.h"
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
#include "LinedPlain.h"

#include "MonoInst.h"
#include "ClassInfo.h"
#include "Refs.h"
#include "CppClass.h"
#include "CSComponent.h"
#include "RenderPassUI.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "TestLightComponent.h"
#include "Material.h"
#include "NavMesh.h"
#include "UI\SceneEditorWindow.h"


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
	Assets::ShaderWorldGride,
	"../../data/engine/shaders/rp_image.hlsl",
	"../../data/engine/shaders/shadow_map.hlsl",
	"../../data/engine/shaders/rp_screen_quad.hlsl",
	"../../data/engine/shaders/screen.hlsl",
};

Game::Game() {
	m_window = new Window();
	m_render = new Render();
	m_input = new InputDevice();
	m_hotkeys = new HotKeys();
	m_shaderAsset = new ShaderAsset();
	m_meshAsset = new MeshAsset();
	m_imageAsset = new ImageAsset();
	m_assets = new Assets();
	m_assetStore = new AssetStore();
	m_ui = new UserInterface();
}

Game::~Game() {
	delete m_window;
	delete m_render;
	delete m_input;
	delete m_hotkeys;
	delete m_shaderAsset;
	delete m_meshAsset;
	delete m_imageAsset;
	delete m_assets;
	delete m_assetStore;
	delete m_ui;
}

void Game::Init(MonoInst* imono) {
	m_InitMono(imono);

	m_window->Init(this, L"CGLab6 - Shadow Map", 1920, 1080);
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

	m_InitImGui();
	m_ui->Init(this);

	m_editorScene = CreateScene(true);

	auto editorWindow = ui()->CreateSceneWindow("Editor", "Editor");
	editorWindow->visible = true;
	editorWindow->scene(m_editorScene);
	
	auto gameWindow = ui()->CreateSceneWindow("Game", "Game");
	gameWindow->visible = true;

}

void Game::m_InitMono(MonoInst* imono) {
	m_mono = imono;

	auto gameType = m_mono->GetType("Engine", "Game");
	auto method_Init = mono::make_method_invoker<void(CppRef)>(gameType, "cpp_Init");

	auto gameRef = CppRefs::Create(this);
	method_Init(CppRef::Create(gameRef.cppRef()));
}

void Game::Run() {
	m_render->Start();
	m_meshAsset->Start();
	
	tmpSceneAssetId = assets()->CreateAssetId();
	///
	auto cppObj = CppClass();
	auto csLink = CSLinked<CppClass>(mono());

	
	
	//isExitRequested = true; // return;
	///
	PushScene(m_editorScene);

	//csLink.Link(cppObj, "EngineMono", "CSClass");
	
	auto light = currentScene()->CreateActor("Light");
	light->localPosition({ 0, 0, 300 });
	light->localRotation({ rad(-45), rad(45 + 180), 0 });
	light->AddComponent<DirectionLight>();
	light->AddComponent<AmbientLight>();

	//currentScene()->CreateActor("GameController")->AddComponent<GameController>();

	auto material = meshAsset()->CreateDynamicMaterial("World Gride", Assets::ShaderWorldGride);

	auto testScene = currentScene()->CreateActor("test scene")->AddComponent<MeshComponent>();
	testScene->mesh(meshAsset()->GetMesh("../../data/assets/models/test_navmesh.obj"));
	testScene->SetMaterial(0, material);

	//inFocus = false;
	//m_lastGameCamera = scene()->mainCamera();
	//m_editorCamera->Attach();
	NavMesh nav_mesh(this);
	auto nav_status = nav_mesh.NavMeshBuild();
	std::cout << "Navmeshbuild status =" << nav_status << std::endl;
	auto ret_find = nav_mesh.FindPath(Vector3(-1160, 0.5, 863), Vector3(1725.460, 0.5, -484.768), 0, 0);
	std::cout << "ret_find " << ret_find << std::endl;
   auto path = nav_mesh.GetPath(0);
   std::cout << "path\n";
   for(const auto& vec: path)
   {
   	std::cout << "x="<<vec.x<<"\ty="<<vec.y<<"\tz="<<vec.z<<'\n';
   }
	PopScene();
		
	MSG msg = {};

	float targetFrameTime = 1.0f / 60.0f;
	float accumFrameTime = targetFrameTime;

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

		accumFrameTime += deltaTime();
		if (accumFrameTime >= targetFrameTime) {
			accumFrameTime -= targetFrameTime;

			m_render->Draw(&m_scenes);
			
			m_updateCounter.Update();
			if (m_updateCounter.HasChanges()) {
				WCHAR text[256];
				swprintf_s(text, TEXT("FPS: %d"), m_updateCounter.FPS());
				SetWindowText(m_window->GetHWindow(), text);
			}
		}

		m_fpsCounter.Update();
		/*if (m_fpsCounter.HasChanges()) {
			WCHAR text[256];
			swprintf_s(text, TEXT("FPS: %d"), m_fpsCounter.FPS());
			SetWindowText(m_window->GetHWindow(), text);
		}*/
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
	
	m_BeginUpdateImGui();

	/// Update
	if (hotkeys()->GetButtonDown(Keys::R))
	{
		MeshComponent::TempVisible = !MeshComponent::TempVisible;
	}
	m_hotkeys->Update(input());

	auto it = m_scenes.begin();
	while (it != m_scenes.end()) {

		auto scene = (*it);
		if (scene->f_isDestroyed) {
			it = m_EraseScene(it);
			continue;
		}
		scene->f_Update();
		it++;
	}

	ui()->Draw();

	static bool once = true;
	if (once) {
		once = false;
		ui()->GetSceneWindow("Editor")->focus();
	}

	/// Post Update
	m_EndUpdateImGui();

	m_hotkeys->LateUpdate();

	if (m_hotkeys->GetButtonDown(Keys::Tilda)) {
		inFocus = !inFocus;
		if (!inFocus) {
			m_editorScene->mainCamera(nullptr);
		}
		else {
			ui()->SelectedActor(nullptr);
			m_editorScene->AttachGameCamera();
		}
	}

	if (m_hotkeys->GetButtonDown(Keys::R) && m_hotkeys->GetButton(Keys::Ctrl)) {
		shaderAsset()->RecompileShaders();
		meshAsset()->ReloadMaterials();
		std::cout << std::endl;
	}

}

void Game::m_Destroy() {

	for (auto it = m_scenes.begin(); it != m_scenes.end(); it = m_EraseScene(it)) 
		(*it)->f_Destroy();

	m_DestroyImGui();
	m_hotkeys->Destroy();
	m_meshAsset->Destroy();
	m_render->Destroy();
	m_window->Destroy();
}

void Game::PushScene(Scene* value) {
	m_sceneStack.push_back(value);
	m_callbacks.setSceneRef(CppRefs::GetRef(value));

	if(!value->m_isStarted)
		value->Start();
}

void Game::PopScene() {
	m_sceneStack.pop_back();
	m_callbacks.setSceneRef(RefCpp(0));
}

Scene* Game::CreateScene(bool isEditor) {
	auto* scene = new Scene();
	m_scenes.push_back(scene);

	scene->f_sceneIter = --m_scenes.end();
	scene->f_ref = CppRefs::Create(scene);
	scene->Init(this, isEditor);

	return scene;
}

void Game::DestroyScene(Scene* scene) {
	for (auto sceneInStack : m_sceneStack) {
		if (sceneInStack == scene)
			throw std::exception("Can't destroy a scene while it's in use");
	}
	scene->f_Destroy();
	m_EraseScene(scene->f_sceneIter);
}

std::list<Scene*>::iterator Game::m_EraseScene(std::list<Scene*>::iterator iter) {
	auto scene = *iter;

	CppRefs::Remove(scene->f_ref);
	auto nextIter = m_scenes.erase(scene->f_sceneIter);
	delete scene;

	return nextIter;
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
	io.KeysDown[io.KeyMap[ImGuiKey_Enter]];
	io.KeysDown[io.KeyMap[ImGuiKey_UpArrow]];
	io.KeysDown[io.KeyMap[ImGuiKey_DownArrow]];

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

void Game::DeleteMaterialFromAllScenes(const Material* material) {
	for (auto scene : m_scenes)
		scene->renderer.UnRegisterMaterial(material);
}

void Game::TogglePlayMode() {
	auto tmpScenePath = "../../Example/Editor/tmp/EditorScene.yml";
	auto assetId = tmpSceneAssetId.c_str();
	
	if (m_gameScene == nullptr) {
		m_gameScene = CreateScene(false);

		auto editorSceneRef = CppRefs::GetRef(m_editorScene);
		auto gameSceneRef = CppRefs::GetRef(m_gameScene);
		
		m_callbacks.saveScene(editorSceneRef, (size_t)assetId, (size_t)tmpScenePath);
		m_callbacks.loadScene(gameSceneRef, (size_t)assetId);

		auto gameWindow = ui()->GetSceneWindow("Game");
		if (gameWindow == nullptr)
			gameWindow = ui()->CreateSceneWindow("Game", "Game");

		gameWindow->scene(m_gameScene);
		gameWindow->visible = true;
		gameWindow->focus();
	}
	else {
		if (ui()->selectedScene() == m_gameScene)
			ui()->selectedScene(nullptr);

		if (ui()->HasActor() && ui()->GetActor()->scene() == m_gameScene)
			ui()->SelectedActor(nullptr);

		DestroyScene(m_gameScene);
		m_gameScene = nullptr;

		auto gameWindow = ui()->GetSceneWindow("Game");
		if (gameWindow != nullptr)
			gameWindow->scene(nullptr);

		auto editorWindow = ui()->GetSceneWindow("Editor");
		editorWindow->focus();
	}
}

DEF_FUNC(Game, SetGameCallbacks, void)(CppRef gameRef, const GameCallbacks& callbacks) {
	CppRefs::ThrowPointer<Game>(gameRef)->callbacks(callbacks);
}

DEF_FUNC(Game, mainCamera_get, CsRef)(CppRef gameRef) {
	auto camera = CppRefs::ThrowPointer<Game>(gameRef)->currentScene()->mainCamera();
	return camera != nullptr ? camera->csRef() : RefCs(0);
}

DEF_FUNC(Game, Exit, void)(CppRef gameRef) {
	CppRefs::ThrowPointer<Game>(gameRef)->Exit(0);
}

DEF_FUNC(Game, PushScene, void)(CppRef gameRef, CppRef sceneRef) {
	auto game = CppRefs::ThrowPointer<Game>(gameRef);
	auto scene = CppRefs::ThrowPointer<Scene>(sceneRef);

	game->PushScene(scene);
}

DEF_FUNC(Game, PopScene, void)(CppRef gameRef) {
	auto game = CppRefs::ThrowPointer<Game>(gameRef);

	game->PopScene();
}

DEF_FUNC(Game, CreateScene, CppRef)(CppRef gameRef, bool isEditor) {
	auto game = CppRefs::ThrowPointer<Game>(gameRef);

	auto scene = game->CreateScene(isEditor);
	return CppRefs::GetRef(scene);
}

DEF_FUNC(Game, DestroyScene, void)(CppRef gameRef, CppRef sceneRef) {
	auto game = CppRefs::ThrowPointer<Game>(gameRef);
	auto scene = CppRefs::ThrowPointer<Scene>(sceneRef);

	game->DestroyScene(scene);
}