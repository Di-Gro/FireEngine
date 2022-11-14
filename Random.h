#pragma once

#ifndef RANDOM_H
#define RANDOM_H

#include <random>
#include <limits>

using namespace std;

class Random {
private:
	mt19937 m_gen;

public:
	Random() {
		random_device rd;
		m_gen = mt19937(rd());
	}

	/// <summary>
	/// Возвращает int в диапазоне [min, max]
	/// </summary>
	int Range(int min, int max) {
		uniform_int_distribution<int> dis(min, max);
		return dis(m_gen);
	}

	int Int() {
		uniform_int_distribution<int> dis((std::numeric_limits<int>::min)(), (numeric_limits<int>::max)());
		return dis(m_gen);
	}

	/// <summary>
	/// Возвращает float значение в диапазоне [0, 1)
	/// </summary>
	float Value() {
		uniform_real_distribution<float> dis(0, 1);
		return dis(m_gen);
	}
};


#endif // !RANDOM_H
