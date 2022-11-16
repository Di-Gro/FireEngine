#include "HotKeys.h"

#include "Game.h"


void HotKeys::Update(InputDevice* input) {
	for (auto it = m_states.begin(); it != m_states.end(); it++) {
		bool hasDown = input->IsKeyDown(it->first);
		auto& state = it->second.state;

		if (hasDown) {
			if (state == KeyState::None || state == KeyState::Release)
				state = KeyState::Press;

			else if (state == KeyState::Press)
				state = KeyState::Hold;
		}
		else {
			if (state == KeyState::Press || state == KeyState::Hold)
				state = KeyState::Release;

			else if (state == KeyState::Release)
				state = KeyState::None;
		}
	}
}

void HotKeys::RegisterHotkey(Keys key) {
	if (m_states.count(key) == 0)
		m_states[key] = { KeyState::None, 1 };
	else
		m_states[key].count++;
}

void HotKeys::UnregisterHotkey(Keys key) {
	auto it = m_states.find(key);

	if (it != m_states.end()) {
		it->second.count--;
		if (it->second.count == 0)
			m_states.erase(key);
	}
}

bool HotKeys::Is(Keys key, KeyState state) {
	if (m_states.count(key) > 0)
		return m_states[key].state == state;

	return false;
}
