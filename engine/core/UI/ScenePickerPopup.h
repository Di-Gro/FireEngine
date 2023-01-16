#pragma once

#include <string>
#include <vector>

#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"

class Scene;

class ScenePickerPopup {
public:
	
	std::string header = "Scene";
	int selected;

	ImGuiPopupFlags flags = ImGuiPopupFlags_MouseButtonRight;

private:
	bool m_isOpen = false;
	int m_selectedIndex = -1;
	std::string m_input;
	int m_filteredCount = 0;
	int m_typeIdHash = 0;

public:
	bool Open(Scene* scene, const char* id);

private:
	static bool IsMatch(std::string source, std::string target);

	bool m_DrawMenu(Scene* scene);
};

