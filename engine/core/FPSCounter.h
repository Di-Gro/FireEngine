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

	const float& GetDelta() { return m_deltaTime; }
	bool IsRateChanged() { return m_hasChanges; }
	int RatePerSecond() { return m_fps; }

	void MakeStep();
};

class FixedTimer : private FPSCounter {
private:
	FPSCounter m_globalTimer;

	float m_targetRate = 60.0f;
	float m_targetFixedTime = 1.0f / m_targetRate;
	float m_accumFixedTime = m_targetFixedTime;

public:
	using FPSCounter::IsRateChanged;
	using FPSCounter::RatePerSecond;

	const float& GetDelta() { return m_targetFixedTime;  }

	float targetRate() { return m_targetRate; }

	void targetRate(float targetRatePerSecond) {
		m_targetRate = targetRatePerSecond;
		m_targetFixedTime = 1.0f / targetRatePerSecond;
		m_accumFixedTime = m_targetFixedTime;
	}

	bool NextStep();
};