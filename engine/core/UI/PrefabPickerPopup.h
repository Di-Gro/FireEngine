#pragma once

#include <string>
#include <vector>

#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"

class Actor;

class PrefabPickerPopup {
public:
	std::string header = "Prefab";
	int selected;

	ImGuiPopupFlags flags;

private:
	bool m_isOpen = false;
	int m_selectedIndex = -1;
	std::string m_input;
	int m_filteredCount = 0;
	int m_prefabIdHash = 0;

public:
	bool Open(Actor* actor);

private:
	static bool IsMatch(std::string source, std::string target);

	bool m_DrawMenu(Actor* actor);
};

