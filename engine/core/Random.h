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
	/// ���������� int � ��������� [min, max]
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
	/// ���������� float �������� � ��������� [0, 1)
	/// </summary>
	float Value() {
		uniform_real_distribution<float> dis(0, 1);
		return dis(m_gen);
	}
};


#endif // !RANDOM_H
