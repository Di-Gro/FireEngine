#pragma once

#include <chrono>


class FPSCounter {
private:
	int m_fps = 0;
	float m_deltaTime = 0;
	float m_totalTime = 0;
	unsigned int m_frameCount = 0;
	bool m_hasChanges = false;

	std::chrono::steady_clock::time_point m_prev;

public:

	FPSCounter() { m_prev = std::chrono::steady_clock::now(); }

	const float& GetDeltaTime() { return m_deltaTime; }
	bool HasChanges() { return m_hasChanges; }
	int FPS() { return m_fps; }

	void Update();
};
