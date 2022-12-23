#include "RenderPassUI.h"

#include <fstream>

#include "imgui\imgui.h"
#include "DirectionLight.h"
#include "AmbientLight.h"
#include "PointLight.h"
#include "SpotLight.h"


DEF_PURE_COMPONENT(RenderPassUI)


void RenderPassUI::OnStart() {
	editor.SetShowWhitespaces(false);
}

void RenderPassUI::OnUpdate() {		
	auto opaquePass = game()->render()->opaquePass();
	auto window = game()->window();

	auto w = (float)window->GetWidth();
	auto h = (float)window->GetHeight();

	//ImGui::Begin("target0Tex: diffuse, spec");
	//ImGui::Image(opaquePass->target0Res.get(), { w / 6, h / 6 });
	//ImGui::End();
	//	
	//ImGui::Begin("target1Tex: normals");
	//ImGui::Image(opaquePass->target1Res.get(), { w / 6, h / 6 });
	//ImGui::End();

	//ImGui::Begin("target2Tex: vertex color");
	//ImGui::Image(opaquePass->target2Res.get(), { w / 6, h / 6 });
	//ImGui::End();

	//ImGui::Begin("target3Tex: world pos");
	//ImGui::Image(opaquePass->target3Res.get(), { w / 6, h / 6 });
	//ImGui::End();

	//ImGui::Begin("targetDs: Depth");
	//ImGui::Image(game()->render()->depthRes()->get(), { w / 6, h / 6 });
	//ImGui::End();

	auto input = game()->hotkeys();
	if (input->GetButtonDown(Keys::T) && input->GetButton(Keys::Ctrl)) {
		m_editorOpened = true;
		game()->inFocus = false;
	}

	if (m_editorOpened && !game()->inFocus) {
		m_DrawTextEditor();
	}
}

void RenderPassUI::OpenFile(const std::string& path) {
	std::ifstream file(path);
	if (file.good())
	{
		std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		editor.SetText(str);

		m_currentPath = path;
	}
}

void RenderPassUI::SaveFile(const std::string& text) {
	std::ofstream file(m_currentPath);
	if (file.good())
	{
		file.write(text.c_str(), text.size());
	}
}

void RenderPassUI::m_DrawTextEditor() {
	auto input = game()->hotkeys();

	auto cpos = editor.GetCursorPosition();
	ImGui::Begin("Shader Editor", &m_editorOpened, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar);
	ImGui::SetWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
		
	m_DrawTextEditorMenu();

	if (editor.isFocused) {
		bool ctrl = input->GetButton(Keys::Ctrl);
		bool shift = input->GetButton(Keys::LeftShift);

		if (input->GetButtonDown(Keys::Backspace)) editor.Backspace();
		if (ctrl && input->GetButtonDown(Keys::C)) editor.Copy();
		if (ctrl && input->GetButtonDown(Keys::V)) editor.Paste();
		if (ctrl && input->GetButtonDown(Keys::X)) editor.Cut();
		if (ctrl && !shift && input->GetButtonDown(Keys::Z)) editor.Undo();
		if (ctrl && shift && input->GetButtonDown(Keys::Z)) editor.Redo();

		if (input->GetButtonDown(Keys::Tab)) editor.InsertText("\t");
		if (input->GetButtonDown(Keys::Enter)) editor.InsertText("\n");

		if (input->GetButtonDown(Keys::Up)) editor.MoveUp();
		if (input->GetButtonDown(Keys::Down)) editor.MoveDown();
		if (input->GetButtonDown(Keys::Right)) editor.MoveRight();
		if (input->GetButtonDown(Keys::Left)) editor.MoveLeft();

		if (ctrl && input->GetButtonDown(Keys::S)) {
			if (m_currentPath != "")
				SaveFile(editor.GetText());
		}
	}
	
	ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s | %s", cpos.mLine + 1, cpos.mColumn + 1, editor.GetTotalLines(),
		editor.IsOverwrite() ? "Ovr" : "Ins",
		editor.CanUndo() ? "*" : " ",
		editor.GetLanguageDefinition().mName.c_str(), m_currentPath);

	bool tilda = input->GetButtonDown(Keys::Tilda);
	if (!tilda)
		editor.Render("TextEditor");

	ImGui::End();
}

void RenderPassUI::m_DrawTextEditorMenu() {
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Shader"))
		{
			for (auto& path : game_shaderPaths) {
				if (ImGui::MenuItem(path.c_str()))
				{
					OpenFile(path);
				}
			}
			ImGui::EndMenu();
		}

		if (m_currentPath != "") {
			if (ImGui::Button("Compile")) {
				SaveFile(editor.GetText());
				game()->SendGameMessage("recompile");
			}
		}

		ImGui::Text("%s", m_currentPath.c_str());

		ImGui::EndMenuBar();
	}
}
