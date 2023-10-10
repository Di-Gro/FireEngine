#pragma once

#include <string>
#include <vector>

#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"

class Game;

class AssetPickerPopup {
public:
	//ImGuiID imguiID;
	std::string header;
	int selected;

	int scriptIdHash = 0;

	ImGuiPopupFlags flags;

private:
	bool m_isOpen = false;
	int m_selectedIndex = -1;
	int m_size = 0;
	std::string m_input;
	std::vector<int> m_emptyContent;

public:
	bool Open(Game* game);

private:
	static bool IsMatch(std::string source, std::string target);


};

