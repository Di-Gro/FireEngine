#pragma once

#include <unordered_map>

//#include "Component.h"
#include "InputDevice.h"


class HotKeys {
private:
	struct MapItem {
		KeyState state = KeyState::None;
		unsigned int count = 0;
	};

	std::unordered_map<Keys, MapItem> m_states;

public:
	void Update(InputDevice* input);

	bool Is(Keys key, KeyState state);
	
	void RegisterHotkey(Keys key);
	void UnregisterHotkey(Keys key);

};

