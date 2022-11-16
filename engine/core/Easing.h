#pragma once
#include "cmath"

namespace Easing {
	float EaseInOutCubic(float x) {
		return x < 0.5 ? 4 * x * x * x : 1 - pow(-2 * x + 2, 3) / 2;
	}

	float EaseOutBack(float x) {
		float c1 = 1.70158f;
		float c3 = c1 + 1;
		return 1 + c3 * pow(x - 1, 3) + c1 * pow(x - 1, 2);
	}

	float EaseOutExpo(float x) {
		return x == 1 ? 1 : 1 - pow(2, -10 * x);
	}
}