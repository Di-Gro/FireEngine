#pragma once

#include <string>
#include <vector>

#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"

class Game; 

class ComponentPicker {
public:
	//std::string imguiID;
	std::string header;
	std::string selected;

	//const std::vector<std::string>* content;

	ImGuiPopupFlags flags;

private:
	bool m_isOpen = false;
	int m_selectedIndex = -1;
	std::string m_input;

public:
	bool Open(Game* game);

	bool isOpen() { return m_isOpen; }

public:
	static void PushPopupStyles();
	static void PopPopupStyles();

private:
	static bool IsMatch(std::string source, std::string target);

};

