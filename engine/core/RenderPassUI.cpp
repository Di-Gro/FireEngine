#include "RenderPassUI.h"

#include <fstream>

#include "imgui\imgui.h"


DEF_PURE_COMPONENT(RenderPassUI)


void RenderPassUI::OnStart() {
	auto opaquePass = game()->render()->opaquePass();

	editor.SetShowWhitespaces(false);
}

void RenderPassUI::OnUpdate() {		
	auto opaquePass = game()->render()->opaquePass();

	ImGui::Begin("target0Tex: diffuse, spec");
	ImGui::Image(opaquePass->target0Res.get(), { 1920 / 6, 1061 / 6 });
	ImGui::End();
		
	ImGui::Begin("target1Tex: normals");
	ImGui::Image(opaquePass->target1Res.get(), { 1920 / 6, 1061 / 6 });
	ImGui::End();

	ImGui::Begin("target2Tex: vertex color");
	ImGui::Image(opaquePass->target2Res.get(), { 1920 / 6, 1061 / 6 });
	ImGui::End();

	ImGui::Begin("target3Tex: world pos");
	ImGui::Image(opaquePass->target3Res.get(), { 1920 / 6, 1061 / 6 });
	ImGui::End();

	ImGui::Begin("targetDs: Depth");
	ImGui::Image(game()->render()->depthRes()->get(), { 1920 / 6, 1061 / 6 });
	ImGui::End();

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

		//if (ImGui::BeginMenu("File"))
		//{
		//	if (ImGui::MenuItem("Save"))
		//	{
		//		auto textToSave = editor.GetText();
		//		SaveFile(textToSave);
		//	}
		//	if (ImGui::MenuItem("Close")) {
		//		m_editorOpened = false;
		//	}
		//	ImGui::EndMenu();
		//}
		//if (ImGui::BeginMenu("Edit"))
		//{
		//	bool ro = editor.IsReadOnly();
		//	if (ImGui::MenuItem("Read-only mode", nullptr, &ro))
		//		editor.SetReadOnly(ro);
		//	ImGui::Separator();

		//	if (ImGui::MenuItem("Undo", "ALT-Backspace", nullptr, !ro && editor.CanUndo()))
		//		editor.Undo();
		//	if (ImGui::MenuItem("Redo", "Ctrl-Y", nullptr, !ro && editor.CanRedo()))
		//		editor.Redo();

		//	ImGui::Separator();

		//	if (ImGui::MenuItem("Copy", "Ctrl-C", nullptr, editor.HasSelection()))
		//		editor.Copy();
		//	if (ImGui::MenuItem("Cut", "Ctrl-X", nullptr, !ro && editor.HasSelection()))
		//		editor.Cut();
		//	if (ImGui::MenuItem("Delete", "Del", nullptr, !ro && editor.HasSelection()))
		//		editor.Delete();
		//	if (ImGui::MenuItem("Paste", "Ctrl-V", nullptr, !ro && ImGui::GetClipboardText() != nullptr))
		//		editor.Paste();

		//	ImGui::Separator();

		//	if (ImGui::MenuItem("Select all", nullptr, nullptr))
		//		editor.SetSelection(TextEditor::Coordinates(), TextEditor::Coordinates(editor.GetTotalLines(), 0));

		//	ImGui::EndMenu();
		//}

		//if (ImGui::BeginMenu("View"))
		//{
		//	if (ImGui::MenuItem("Dark palette"))
		//		editor.SetPalette(TextEditor::GetDarkPalette());
		//	if (ImGui::MenuItem("Light palette"))
		//		editor.SetPalette(TextEditor::GetLightPalette());
		//	if (ImGui::MenuItem("Retro blue palette"))
		//		editor.SetPalette(TextEditor::GetRetroBluePalette());
		//	ImGui::EndMenu();
		//}

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