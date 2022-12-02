#include "SimpleMath.h"

std::string ToString(const Vector3& vec) {
	return "("
		+ std::to_string(vec.x) + ", "
		+ std::to_string(vec.y) + ", "
		+ std::to_string(vec.z) + ")";
}

std::string ToString(const Quaternion& vec) {
	return "("
		+ std::to_string(vec.x) + ", "
		+ std::to_string(vec.y) + ", "
		+ std::to_string(vec.z) + ", " 
		+ std::to_string(vec.w) + ")";
}

inline void FixNaN(Vector3& value) {
	if (value.x != value.x) value.x = 0;
	if (value.y != value.y) value.y = 0;
	if (value.z != value.z) value.z = 0;
}

inline void FixNaN(Quaternion& value) {
	if (value.x != value.x) value.x = 0;
	if (value.y != value.y) value.y = 0;
	if (value.z != value.z) value.z = 0;
	if (value.w != value.w) value.w = 0;
}

inline void FixNaN(float& value) {
	if (value != value)
		value = 0;
}

inline bool IsNaN(float& value) {
	return value != value;
}

inline bool IsAllNaN(Matrix& value) {
	return
		IsNaN(value._11) && IsNaN(value._12) && IsNaN(value._13) &&
		IsNaN(value._21) && IsNaN(value._22) && IsNaN(value._23) &&
		IsNaN(value._31) && IsNaN(value._32) && IsNaN(value._33) &&
		IsNaN(value._41) && IsNaN(value._42) && IsNaN(value._43);
}

inline void FixNaN(Matrix& value) {
	if (IsAllNaN(value)) {
		value = Matrix::Identity * Matrix::CreateScale({ 0, 0, 0 });
		return;
	}
	FixNaN(value._11);
	FixNaN(value._12);
	FixNaN(value._13);

	FixNaN(value._21);
	FixNaN(value._22);
	FixNaN(value._23);

	FixNaN(value._31);
	FixNaN(value._32);
	FixNaN(value._33);

	FixNaN(value._41);
	FixNaN(value._42);
	FixNaN(value._43);
}

inline bool IsNaN(Matrix& value) {
	return IsAllNaN(value);
}