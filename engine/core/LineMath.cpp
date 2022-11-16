#include "LineMath.h"


static inline float rad(float angle) {
	static float pi = 3.14159265f;
	return angle * pi / 180;
}

static inline float deg(float radians) {
	static float pi = 3.14159265f;
	return radians * 180 / pi;;
}

bool LineMath::Intersection(
	const Vector2& p11, 
	const Vector2& p12, 
	const Vector2& p21, 
	const Vector2& p22, 
	Vector2& intersect)
{
	intersect = Vector2(0, 0);
	float dx1 = p11.x - p12.x;
	float dx2 = p21.x - p22.x;
	float dy1 = p11.y - p12.y;
	float dy2 = p21.y - p22.y;
	float d = dx1 * dy2 - dx2 * dy1;
	if (d > 0 || d < 0)
	{
		float x1 = p11.x * p12.y - p11.y * p12.x;
		float x2 = p21.x * p22.y - p21.y * p22.x;
		float x = x1 * dx2 - x2 * dx1;
		float y = x1 * dy2 - x2 * dy1;
		intersect = Vector2(x / d, y / d);
		return true;
	}
	return false;
}

bool LineMath::IntersectionLineWithSegment(
	const Vector2& p11,
	const Vector2& p12,
	const Vector2& s21,
	const Vector2& s22,
	Vector2& intersect)
{
	intersect = Vector2(0, 0);
	if (Intersection(p11, p12, s21, s22, intersect))
	{
		float dot = (intersect - s21).Dot(s22 - s21);
		if (dot < 0)
			return false;
		dot = (intersect - s22).Dot(s21 - s22);
		if (dot < 0)
			return false;
		return true;
	}
	return false;
}

bool LineMath::IntersectionLineSegments(
	const Vector2& s11,
	const Vector2& s12,
	const Vector2& s21,
	const Vector2& s22,
	Vector2& intersect)
{
	intersect = Vector2(0, 0);
	if (Intersection(s11, s12, s21, s22, intersect))
	{
		float dot = (intersect - s11).Dot(s12 - s11);
		if (dot < 0)
			return false;

		dot = (intersect - s12).Dot(s11 - s12);
		if (dot < 0)
			return false;

		dot = (intersect - s21).Dot(s22 - s21);
		if (dot < 0)
			return false;

		dot = (intersect - s22).Dot(s21 - s22);
		if (dot < 0)
			return false;

		return true;
	}
	return false;
}

bool LineMath::IsClamped(
	const Vector2& p,
	const Vector2& s1,
	const Vector2& s2)
{
	float dot = (p - s1).Dot(s2 - s1);
	if (dot < 0)
		return false;
	dot = (p - s2).Dot(s1 - s2);
	if (dot < 0)
		return false;
	return true;
}

bool LineMath::IsIntersectedSegments(
	const Vector2& p1,
	const Vector2& p2,
	const Vector2& m1,
	const Vector2& m2)
{
	float a = PseudoDotProduct(m2, p1, p2);
	float b = PseudoDotProduct(m1, p1, p2);
	if (a * b >= 0)
		return false;

	a = PseudoDotProduct(p2, m1, m2);
	b = PseudoDotProduct(p1, m1, m2);
	if (a * b >= 0)
		return false;

	return true;
}

float LineMath::PseudoDotProduct(
	const Vector2& p0,
	const Vector2& p1,
	const Vector2& p2) 
{
	return (p2.x - p1.x) * (p0.y - p1.y) - (p2.y - p1.y) * (p0.x - p1.x);
}

float LineMath::Angle(const Vector2& v1, const Vector2& v2) {
	static float pi = 3.14159265f;

	auto v1n = v1;
	auto v2n = v2;
	v1n.Normalize();
	v2n.Normalize();

	float radians = acos(v1n.Dot(v2n));
	return radians * 180 / pi;;
}

float LineMath::SignedAngle(const Vector2& v1, const Vector2& v2) {
	float pdp = LineMath::PseudoDotProduct(Vector2(0, 0), v1, v2);
	float angle = Angle(v1, v2);
	return pdp >= 0 ? angle : -angle;
}

Vector2 LineMath::Rotate(const Vector2& v, float angle) {
	Vector2 point;
	point.x = v.x * cos(rad(angle)) - v.y * sin(rad(angle));
	point.y = v.x * sin(rad(angle)) + v.y * cos(rad(angle));
	return point;
}

Vector2 LineMath::ClosestPointOnLine(const Vector2& p0, const Vector2& s1, const Vector2& s2)
{
	Vector2 lineVec = s2 - s1;
	Vector2 pointVec = p0 - s1;

	Vector2 nLineVec = lineVec; nLineVec.Normalize();
	float dot = pointVec.Dot(nLineVec);
	Vector2 p = s1 + dot * nLineVec;

	return p;
}

Vector2 LineMath::ClosestPointOnLineSegment(const Vector2& p0, const Vector2& s1, const Vector2& s2)
{
	float dot = (p0 - s2).Dot(s1 - s2);
	if (dot < 0)
		return s2;

	Vector2 lineVec = s2 - s1;
	Vector2 pointVec = p0 - s1;
	dot = pointVec.Dot(lineVec);
	if (dot < 0)
		return s1;

	Vector2 nLineVec = lineVec; nLineVec.Normalize();

	dot = pointVec.Dot(nLineVec);
	Vector2 p = s1 + dot * nLineVec;

	return p;
}

bool LineMath::IsRectContainsPoint(
	const Vector2& r0,
	const Vector2& r1,
	const Vector2& r2,
	const Vector2& r3,
	const Vector2& p0) 
{
	if (PseudoDotProduct(p0, r0, r1) < 0)
		return false;

	if (PseudoDotProduct(p0, r1, r2) < 0)
		return false;

	if (PseudoDotProduct(p0, r2, r3) < 0)
		return false;

	if (PseudoDotProduct(p0, r3, r0) < 0)
		return false;

	return true;
}

//bool LineMath::linePlaneIntersection(
//	Vector3* contact,
//	const Vector3& rayDirection,
//	const Vector3& rayOrigin,
//	const Vector3& planeNormal,
//	const Vector3& planePoint)
//{
//	// get d value
//	float d = planeNormal.Dot(planePoint);
//	if (planeNormal.Dot(rayDirection) == 0) {
//		return false; // No intersection, the line is parallel to the plane
//	}
//	// Compute the X value for the directed line rayDirection intersecting the plane
//	float x = (d - planeNormal.Dot(rayOrigin)) / planeNormal.Dot(rayDirection);
//	// output contact point
//	*contact = rayOrigin + rayDirection.Normalized() * x; //Make sure your rayDirection vector is normalized
//	return true;
//}