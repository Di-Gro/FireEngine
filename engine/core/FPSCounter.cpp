#include "FPSCounter.h"

void FPSCounter::MakeStep() {
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

bool FixedTimer::NextStep() {
	if (m_globalTimer.GetDelta() > 1) {
		m_accumFixedTime = m_targetFixedTime;
		m_globalTimer.MakeStep();
	}

	if (m_accumFixedTime >= m_targetFixedTime) {
		m_accumFixedTime -= m_targetFixedTime;
		this->MakeStep();
		return true;
	}
	
	m_globalTimer.MakeStep();

	m_accumFixedTime += m_globalTimer.GetDelta();
	if (m_accumFixedTime >= m_targetFixedTime) {
		m_accumFixedTime -= m_targetFixedTime;
		this->MakeStep();
		return true;
	}
	return false;
}
