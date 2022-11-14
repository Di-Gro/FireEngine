#include "SimpleMath.h"

std::string ToString(const Vector3& vec) {
	return "("
		+ std::to_string(vec.x) + ", "
		+ std::to_string(vec.y) + ", "
		+ std::to_string(vec.z) + ")";
}