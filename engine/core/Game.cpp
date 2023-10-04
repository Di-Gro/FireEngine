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
#include "Physics.h"
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
#include "NavMesh.h"

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

#include "SimpleMath.h"

#include "ContextMenu.h"

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
	"../../engine/data/shaders/rp_image.hlsl",
	"../../engine/data/shaders/shadow_map.hlsl",
	"../../engine/data/shaders/rp_screen_quad.hlsl",
	"../../engine/data/shaders/screen.hlsl",
};

Game::Game() {
	m_window = new Window();
	m_render = new Render();
	m_physics = new Physics();
	m_input = new InputDevice();
	m_hotkeys = new HotKeys();
	m_shaderAsset = new ShaderAsset();
	m_meshAsset = new MeshAsset();
	m_imageAsset = new ImageAsset();
	m_assets = new Assets();
	m_assetStore = new AssetStore();
	m_ui = new UserInterface();

	m_NavMesh = new NavMesh();
}

Game::~Game() {
	delete m_window;
	delete m_render;
	delete m_physics;
	delete m_input;
	delete m_hotkeys;
	delete m_shaderAsset;
	delete m_meshAsset;
	delete m_imageAsset;
	delete m_assets;
	delete m_assetStore;
	delete m_ui;

	delete m_NavMesh;
}

void Game::Init(MonoInst* imono) {
	m_InitMono(imono);

	m_window->Init(this, L"FireEngine", 1920, 1080);
	m_window->Create();

	m_render->Init(this, m_window);
	m_physics->Init(this);
	m_shaderAsset->Init(m_render);
	m_meshAsset->Init(this);
	m_imageAsset->Init();
	m_input->Init(this);
	m_hotkeys->Init(this);
	m_assets->Init(this);
	m_assetStore->Init(this);
	m_NavMesh->Init(this);

	for (auto& path : game_shaderPaths)
		m_shaderAsset->CompileShader(path);

	m_InitImGui();
	m_ui->Init(this);

	auto assetStoreRef = CppRefs::Create((void*)m_assetStore).cppRef();
	callbacks().setAssetStoreRef(RefCpp(assetStoreRef));
	callbacks().loadAssetStore();

	m_editorWindow = ui()->CreateSceneWindow("Editor");
	m_editorWindow->visible = true;

	m_gameWindow = ui()->CreateSceneWindow("Game");
	m_gameWindow->visible = false;

	//tmpSceneAssetId = assets()->CreateTmpAssetId();
}

void Game::m_InitMono(MonoInst* imono) {
	m_mono = imono;

	auto gameType = m_mono->GetType("Engine", "Game");
	auto method_Init = mono::make_method_invoker<void(CppRef)>(gameType, "cpp_Init");

	auto gameRef = CppRefs::Create(this);
	method_Init(CppRef::Create(gameRef.cppRef()));
}

bool Game::LoadScene(Scene* targetScene, int assetGuidHash) {
	bool loaded = false;
	auto sceneRef = CppRefs::GetRef(targetScene);

	if (targetScene->IsAsset()) {
		bool hasAsset = callbacks().hasAssetInStore(targetScene->assetIdHash());
		if (hasAsset)
			loaded = assets()->Load(targetScene->assetIdHash(), sceneRef);
	}
	else if(assetGuidHash != 0) {
		loaded = callbacks().loadScene(sceneRef, assetGuidHash);
	}
	return loaded;
}

#define PEEK_MESSAGE(msg, m_onExit, isExitRequested)\
while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {\
	TranslateMessage(&msg);\
	DispatchMessage(&msg);\
}\
if (msg.message == WM_QUIT || m_onExit) {\
	isExitRequested = true;\
	break;\
}\

void Game::Run() {
	m_render->Start();
	m_meshAsset->Start();

	m_editorScene = CreateScene(true, editorSettings.startupSceneId);
	if (!LoadScene(m_editorScene)) {
		PushScene(m_editorScene);
		m_editorScene->name("!!!This scene can not be saved!!!");
		SceneMenu::AddLight(m_editorScene);
		PopScene();
	}
	m_editorWindow->scene(m_editorScene);

	//PushScene(m_editorScene);

	//auto meshcomp = currentScene()->CreateActor("Half Sphere")->AddComponent<MeshComponent>(true);
	//auto form = Forms4::HalfSphereLined(0.5f, 12, 12);
	//meshcomp->AddShape(&form.verteces, &form.indexes);
	//meshcomp->mesh()->topology = form.topology;
	//meshcomp->meshScale = {10,10,10};
	//
	//currentScene()->CreateActor("GameController")->AddComponent<GameController>();
	//PopScene();

	MSG msg = {};

	m_fixedTimer.targetRate(60.0f);

	bool isExitRequested = false;
	while (!isExitRequested) {
		PEEK_MESSAGE(msg, m_onExit, isExitRequested);
		m_ShowFPS();

		m_updateTimer.MakeStep();

		m_BeginUpdate();
		m_ForScenes(&Scene::f_Update);
		m_DrawUI();

		while (m_fixedTimer.NextStep())
			m_ForScenes(&Scene::f_FixedUpdate);

		m_EndUpdate();
		m_render->Draw(&m_scenes);
	}

	m_Destroy();
}

void Game::m_ShowFPS() {
	if (m_updateTimer.IsRateChanged() || m_fixedTimer.IsRateChanged()) {
		WCHAR text[256];
		swprintf_s(text,
			TEXT("Update Rate: %d\t Fixed Rate: %d"),
			m_updateTimer.RatePerSecond(),
			m_fixedTimer.RatePerSecond()
		);
		SetWindowText(m_window->GetHWindow(), text);
	}
}

void Game::Exit(int code) {
	if (!m_onExit) {
		m_onExit = true;
		m_window->Exit(code);
	}
}

void Game::m_ForScenes(void (Scene::* method)()) {

	auto it = m_scenes.begin();
	while (it != m_scenes.end()) {

		auto scene = (*it);
		if (scene->IsDestroyed()) {
			scene->timeToDestroy--;
			if (scene->timeToDestroy <= 0)
				it = m_EraseScene(it);
			continue;
		}
		(scene->*method)();
		assert(m_sceneStack.size() == 0);

		it++;
	}
}

void Game::m_DrawUI() {
	if (ui()->selectedScene() != nullptr && ui()->selectedScene()->IsDestroyed())
		ui()->selectedScene(nullptr);

	if (ui()->HasActor() && ui()->GetActor()->IsDestroyed())
		ui()->SelectedActor(nullptr);

	ui()->Draw();
}

void Game::m_BeginUpdate() {
	GameUpdateData updateData;
	updateData.deltaTime = deltaTime();
	updateData.deltaFixedTime = deltaFixedTime();
	m_callbacks.setUpdateData(updateData);

	m_hotkeys->Update(input());

	m_BeginUpdateImGui();
	
	if (m_nextScene != nullptr) {
		bool isSelected = ui()->selectedScene() == m_editorScene;

		if (ui()->HasActor() && ui()->GetActor()->scene() == m_editorScene)
			ui()->SelectedActor(nullptr);

		DestroyScene(m_editorScene);
		m_editorScene = m_nextScene;
		m_nextScene = nullptr;

		m_editorWindow->scene(m_editorScene);

		if (isSelected)
			ui()->selectedScene(m_editorScene);

		LoadScene(m_editorScene);
	}
}

void Game::m_EndUpdate() {
	m_EndUpdateImGui();

	m_hotkeys->LateUpdate();

	if (m_hotkeys->GetButtonDown(Keys::Tilda) || m_hotkeys->GetButtonDownEd(Keys::Tilda))
		ToggleGameFocus();

	if (m_hotkeys->GetButtonDownEd(Keys::R, Keys::Ctrl)) {
		shaderAsset()->RecompileShaders();
		meshAsset()->ReloadMaterials();
		std::cout << std::endl;
	}

	if (m_hotkeys->GetButtonDownEd(Keys::S, Keys::LeftShift, Keys::Ctrl)) {
		/// TODO: Сохранить все ассеты
	}
}


void Game::m_Destroy() {

	auto it = m_scenes.begin();
	while (it != m_scenes.end()) {
		DestroyScene(*it);
		it = m_EraseScene(it);
	}

	m_DestroyImGui();
	m_hotkeys->Destroy();
	m_meshAsset->Destroy();
	m_physics->Destroy();
	m_render->Destroy();
	m_window->Destroy();
}

void Game::PushScene(Scene* value) {
	m_sceneStack.push_back(value);
	callbacks().setSceneRef(CppRefs::GetRef(value));

	if(!value->m_isStarted)
		value->Start();
}

void Game::PopScene() {
	m_sceneStack.pop_back();
	callbacks().setSceneRef(RefCpp(0));
}

Scene* Game::CreateScene(bool isEditor) {
	auto* scene = new Scene();
	m_scenes.push_back(scene);

	scene->f_ref = CppRefs::Create(scene);
	scene->f_sceneIter = --m_scenes.end();
	scene->Init(this, isEditor);

	return scene;
}

Scene* Game::CreateScene(bool isEditor, const std::string& assetId) {
	auto gameRef = CppRefs::GetRef(this);
	auto assetIdHash = assets()->GetCsAssetIDHash(assetId);

	auto sceneRef = Scene_PushAsset(gameRef, assetId.c_str(), assetIdHash);
	auto scene = CppRefs::ThrowPointer<Scene>(sceneRef);

	m_scenes.push_back(scene);

	scene->f_sceneIter = --m_scenes.end();
	scene->Init(this, isEditor);

	return scene;
}

void Game::DestroyScene(Scene* scene) {
	if (scene->BeginDestroy()) {
		for (auto sceneInStack : m_sceneStack) {
			if (sceneInStack == scene)
				throw std::exception("Can't destroy a scene while it's in use");
		}
		scene->Destroy();
		scene->EndDestroy();
	}
}

bool  Game::CanChangeScene() { 
	return m_nextScene == nullptr; 
}

void Game::ChangeScene(Scene* scene) {
	if (m_editorScene == scene)
		return;

	m_nextScene = scene;
}

std::list<Scene*>::iterator Game::m_EraseScene(std::list<Scene*>::iterator iter) {
	auto scene = *iter;

	if (scene->IsAsset())
		assets()->Pop(scene->assetIdHash());
	else
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
	auto tmpScenePath = assetStore()->editorPath() + "/Ignore/editor_scene.yml";
	//auto assetId = tmpSceneAssetId.c_str();

	if (m_gameScene == nullptr) {
		m_gameScene = CreateScene(false);

		auto editorSceneRef = CppRefs::GetRef(m_editorScene);
		auto gameSceneRef = CppRefs::GetRef(m_gameScene);

		int assetGuidHash = callbacks().saveScene(editorSceneRef, (size_t)tmpScenePath.c_str());
		bool wasLoaded = false;

		if (assetGuidHash != 0) {
			wasLoaded = LoadScene(m_gameScene, assetGuidHash);
			if (wasLoaded) {
				m_gameScene->name(m_editorScene->name() + " (Game)");

				m_gameWindow->scene(m_gameScene);
				m_gameWindow->visible = true;
				m_gameWindow->focus();

				ui()->SelectedActor(nullptr);

				if (!inFocus)
					ToggleGameFocus();
				PushScene(m_gameScene);
				m_NavMesh->NavMeshBuild();
				PopScene();
			}
		}
		if (!wasLoaded)
			DestroyScene(m_gameScene);
	}
	else {
		if (ui()->selectedScene() == m_gameScene)
			ui()->selectedScene(nullptr);

		if (ui()->HasActor() && ui()->GetActor()->scene() == m_gameScene)
			ui()->SelectedActor(nullptr);

		DestroyScene(m_gameScene);
		m_gameScene = nullptr;

		m_gameWindow->scene(nullptr);
		m_gameWindow->visible = false;

		m_editorWindow->focus();
		m_editorWindow->visible = true;

		m_editorScene->mainCamera(nullptr);

		if (inFocus)
			ToggleGameFocus();
	}
}

void Game::ToggleGameFocus() {
	inFocus = !inFocus;
	auto scene = ui()->selectedScene();
	if (scene != nullptr) {
		if (!inFocus) {
			scene->mainCamera(nullptr);
		}
		else {
			ui()->SelectedActor(nullptr);
			scene->AttachPlayerCamera();
		}

		//if (inFocus && m_gameScene != nullptr) {
		//	auto vpos = m_gameWindow->viewportPosition();
		//	auto vsize = m_gameWindow->viewportSize();

		//	window()->ClipCursor(vpos.x, vpos.y, vsize.x, vsize.y);
		//}
		//else if (window()->IsCursorClipped()) {
		//	window()->UnclipCursor();
		//}
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