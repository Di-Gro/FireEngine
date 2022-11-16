#pragma once

static float pi = 3.14159265f;

inline float clamp(const float& value, const float& min, const float& max) {
	if (value < min)
		return min;
	if (value > max)
		return max;
	return value;
}

inline float rad(const float& angle) {
	return angle * pi / 180;
}

inline float deg(const float& radians) {
	return radians * 180 / pi;;
}

