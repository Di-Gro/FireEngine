#include "UserInterface.h"
#include "SceneEditorWindow.h"

#include "../Game.h"

bool UserInterface::opt_fullscreen = true;
bool UserInterface::dockspaceOpen = true;
bool UserInterface::opt_padding = false;
ImGuiDockNodeFlags UserInterface::dockspace_flags = ImGuiDockNodeFlags_None;

UserInterface::UserInterface() { }

UserInterface::~UserInterface() { }

void UserInterface::Init(Game* game) {
	_game = game;

	m_InitStyles();
	m_InitIcons();

	uiEditor.Init(_game);
	uiHierarchy.Init(_game);
	uiBarMenu.Init(_game);

	uiInspector.Init(_game, this);

	InitMono();
}

void UserInterface::Destroy() {
	for (auto hash_window : m_sceneWindows)
		delete hash_window.second;

	m_sceneWindows.clear();
}

void UserInterface::InitDockSpace()
{
	if (opt_fullscreen)
	{
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	}
	else
	{
		dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
	}

	if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
		window_flags |= ImGuiWindowFlags_NoBackground;

	if (!opt_padding)
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
}

void UserInterface::m_InitStyles() {
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
}

void UserInterface::m_InitIcons() {
	_game->imageAsset()->InitImage(&m_imgMove, "../../data/engine/icons/ic_move.png");
	_game->imageAsset()->InitImage(&m_imgRotate, "../../data/engine/icons/ic_rotate.png");
	_game->imageAsset()->InitImage(&m_imgScale, "../../data/engine/icons/ic_scale.png");
	_game->imageAsset()->InitImage(&m_imgPickup, "../../data/engine/icons/ic_pickup.png");
	_game->imageAsset()->InitImage(&m_imgPickupActor, "../../data/engine/icons/ic_pickup_actor.png");
	_game->imageAsset()->InitImage(&m_imgPickupComponent, "../../data/engine/icons/ic_pickup_component.png");
	_game->imageAsset()->InitImage(&m_imgPickupAsset, "../../data/engine/icons/ic_pickup_asset.png");

	m_texMove = Texture::CreateFromImage(_game->render(), &m_imgMove);
	m_texRotate = Texture::CreateFromImage(_game->render(), &m_imgRotate);
	m_texScale = Texture::CreateFromImage(_game->render(), &m_imgScale);
	m_texPickup = Texture::CreateFromImage(_game->render(), &m_imgPickup);
	m_texPickupActor = Texture::CreateFromImage(_game->render(), &m_imgPickupActor);
	m_texPickupComponent = Texture::CreateFromImage(_game->render(), &m_imgPickupComponent);
	m_texPickupAsset = Texture::CreateFromImage(_game->render(), &m_imgPickupAsset);

	icMove = ShaderResource::Create(&m_texMove);
	icRotate = ShaderResource::Create(&m_texRotate);
	icScale = ShaderResource::Create(&m_texScale);
	icPickup = ShaderResource::Create(&m_texPickup);
	icPickupActor = ShaderResource::Create(&m_texPickupActor);
	icPickupComponent = ShaderResource::Create(&m_texPickupComponent);
	icPickupAsset = ShaderResource::Create(&m_texPickupAsset);
}

void UserInterface::Draw()
{
	InitDockSpace();

	ImGui::Begin("DockSpace Demo", NULL, window_flags);
	if (!opt_padding)
		ImGui::PopStyleVar();

	if (opt_fullscreen)
		ImGui::PopStyleVar(2);

	// Submit the DockSpace
	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
	{
		ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
	}

	// Begin Menu Bar
	uiBarMenu.Draw_UI_BarMenu();
	// Upper Menu
	//uiUpperMenu.Draw_UI_UpperMenu();
	// Hierarchy
	uiHierarchy.Draw_UI_Hierarchy();
	//// Play Game
	//uiPlayGame.Draw_UI_PlayGame();
	// Editor
	//uiEditor.Draw_UI_Editor();
	//// Inspector
	uiInspector.Draw_UI_Inspector();
	//// Content Browser
	//uiContentBrowser.Draw_UI_ContentBrowser();
	//// Console
	//uiConsole.Draw_UI_Console();

	auto dockSpaceId = ImGui::GetID("MyDockSpace");
	auto node = ImGui::DockBuilderGetNode(dockSpaceId);
	auto centralNodeId = node->CentralNode->ID;
	
	for (auto hash_window : m_sceneWindows) {
		auto window = hash_window.second;
		ImGui::DockBuilderDockWindow(window->windowId().c_str(), centralNodeId);
		window->Draw();
	}

	ImGui::End();

}


void UserInterface::SelectedActor(Actor* actor) {
	_actor = actor;
}

bool UserInterface::HasActor() {
	if (_actor != nullptr)
		return true;
	return false;
}

void UserInterface::SetActorActive() {
	_isActorActive = true;
}

bool UserInterface::isActive() {
	if (_isActorActive)
		return true;
	return false;
}

void UserInterface::SetCallbacks(const Callbacks& callbacks) {
	_callbacks = callbacks;
}

void UserInterface::InitMono() {
	auto type = _game->mono()->GetType("Engine", "UserInterface");
	auto method = mono::make_method_invoker<void(CppRef)>(type, "cpp_Init");
	auto cppRefs = CppRefs::Create(this);
	method(CppRef::Create(cppRefs.cppRef()));
}

bool UserInterface::HasSceneWindow(const std::string& sceneId) {
	auto hash = m_StringHash(sceneId);
	return m_sceneWindows.contains(hash);
}

SceneWindow* UserInterface::GetSceneWindow(const std::string& sceneId) {
	auto hash = m_StringHash(sceneId);

	if (m_sceneWindows.contains(hash))
		return m_sceneWindows.at(hash);

	return nullptr;
}

SceneWindow* UserInterface::CreateSceneWindow(const std::string& sceneId, const std::string& name) {
	auto hash = m_StringHash(sceneId);

	if (!m_sceneWindows.contains(hash)) {
		auto window = new SceneEditorWindow(sceneId);
		window->Init(_game);
		window->name = name;

		m_sceneWindows.insert(std::make_pair(hash, window));
	}
	return m_sceneWindows.at(hash);
}

void UserInterface::RemoveSceneWindow(const std::string& sceneId) {
	auto hash = m_StringHash(sceneId);

	if (m_sceneWindows.contains(hash))
		m_sceneWindows.erase(hash);
}

DEF_FUNC(UserInterface, SetCallbacks2, void)(CppRef cppRef, const UserInterface::Callbacks& callbacks) {
	auto component = CppRefs::ThrowPointer<UserInterface>(cppRef);
	component->SetCallbacks(callbacks);
}