#include "UserInterface.h"
#include "../Game.h"

bool UserInterface::opt_fullscreen = true;
bool UserInterface::dockspaceOpen = true;
bool UserInterface::opt_padding = false;
ImGuiDockNodeFlags UserInterface::dockspace_flags = ImGuiDockNodeFlags_None;

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
	// Editor
	uiEditor.Draw_UI_Editor();
	//// Play Game
	uiPlayGame.Draw_UI_PlayGame();
	//// Inspector
	uiInspector.Draw_UI_Inspector();
	//// Content Browser
	uiContentBrowser.Draw_UI_ContentBrowser();
	//// Console
	uiConsole.Draw_UI_Console();

	ImGui::End();
}

void UserInterface::Init(Game* game)
{
	_game = game;
	uiEditor.Init(_game);
	uiHierarchy.Init(_game);
	uiBarMenu.Init(_game);
	uiInspector.Init(_game);
	uiInspector.InitUI(this);
	uiHierarchy.InitUI(this);

	InitMono();
}

void UserInterface::SelectedActor(Actor* actor)
{
	_actor = actor;
}

bool UserInterface::HasActor()
{
	if (_actor != nullptr)
		return true;
	return false;
}

void UserInterface::SetActorActive()
{
	_isActorActive = true;
}

bool UserInterface::isActive()
{
	if (_isActorActive)
		return true;
	return false;
}

void UserInterface::SetCallbacks(const Callbacks& callbacks)
{
	_callbacks = callbacks;
}

void UserInterface::InitMono()
{
	auto type = _game->mono()->GetType("Engine", "UserInterface");
	auto method = mono::make_method_invoker<void(CppRef)>(type, "cpp_Init");
	auto cppRefs = CppRefs::Create(this);
	method(CppRef::Create(cppRefs.cppRef()));
}

//DEF_FUNC(UserInterface, SetCallbacks, void)(CppRef cppRef, const UserInterface::Callbacks& callbacks) {
//	auto component = CppRefs::ThrowPointer<UserInterface>(cppRef);
//	component->SetCallbacks(callbacks);
//}

DEF_FUNC(UserInterface, SetCallbacks, void)(CppRef cppRef, void(*func)(CsRef, float)) {
	auto component = CppRefs::ThrowPointer<UserInterface>(cppRef);
	UserInterface::Callbacks callbacks;
	callbacks.onDrawComponent = func;
	component->SetCallbacks(callbacks);
}