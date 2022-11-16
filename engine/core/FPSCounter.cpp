#include "FPSCounter.h"

void FPSCounter::Update() {
	auto curTime = std::chrono::steady_clock::now();
	m_deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(curTime - m_prev).count() / 1000000.0f;
	m_prev = curTime;

	m_totalTime += m_deltaTime;
	m_frameCount++;

	m_hasChanges = false;
	if (m_totalTime > 1.0f) {
		m_fps = m_frameCount / m_totalTime;
		m_totalTime -= 1.0f;
		m_hasChanges = true;
		m_frameCount = 0;
	}
}
