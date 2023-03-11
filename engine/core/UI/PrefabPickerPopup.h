#pragma once

#include <string>
#include <vector>
#include <functional>

#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"

class Actor;

class PrefabPickerPopup {
private:
	struct MenuAction {
		const char* label;
		bool condition;
		const std::function<void()> action;

		MenuAction(
			const char* label,
			bool condition,
			const std::function<void()>& action
		) : label(label) , condition(condition) , action(action) { }
	};

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
	std::vector<int> m_emptyContent;

public:
	bool Open(Actor* actor);

private:
	static bool IsMatch(std::string source, std::string target);

	void m_DrawMenu(Actor* actor);
	void m_DrawMenuActions(int count, MenuAction* actions);
};

