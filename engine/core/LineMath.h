#pragma once

#include <d3d11.h>
#include <SimpleMath.h>

using namespace DirectX::SimpleMath;


namespace LineMath {

	/// <summary>
	/// »щет точку пересечени€ заданных пр€мых.
	/// ¬озвращает true или false, если пересечение есть или нет, соответственно.
	/// </summary>
	bool Intersection(
		const Vector2& p11,
		const Vector2& p12,
		const Vector2& p21,
		const Vector2& p22,
		Vector2& intersect);

	/// <summary>
	/// »щет точку пересечени€ между заданной пр€мой и отрезком.
	/// ¬озвращает true или false, если пересечение есть или нет, соответственно.
	/// </summary>
	bool IntersectionLineWithSegment(
		const Vector2& p11,
		const Vector2& p12,
		const Vector2& s21,
		const Vector2& s22,
		Vector2& intersect);

	bool IntersectionLineSegments(
		const Vector2& s11,
		const Vector2& s12,
		const Vector2& s21,
		const Vector2& s22,
		Vector2& intersect);

	/// <summary>
	/// ќпредел€ет находитс€ ли точка в пределах отрезка, в т.ч. сверху или снизу.
	/// </summary>
	bool IsClamped(
		const Vector2& p,
		const Vector2& s1,
		const Vector2& s2);

	/// <summary> 
	/// ќпредел€ет пересекаютс€ ли заданные отрезки.
	/// </summary>
	bool IsIntersectedSegments(
		const Vector2& p1,
		const Vector2& p2,
		const Vector2& m1,
		const Vector2& m2);

	/// <summary>
	///  ѕринимает точку, начальную и конечную координату линии.
	///  ¬озвращает косое произведение между заданными лини€ми и точкой.
	///  result < 0 - точка лежит под линией (справа)
	///  result > 0 - точка лежит над линией (слева)
	///  result = 0 - точка лежит на линии
	/// </summary>
	float PseudoDotProduct(
		const Vector2& p0,
		const Vector2& p1,
		const Vector2& p2);

	float Angle( const Vector2& v1, const Vector2& v2);
	float SignedAngle(const Vector2& v1, const Vector2& v2);

	Vector2 Rotate(const Vector2& v, float angle);

	/// <summary>
	/// ѕринимает точку, начальную и конечную координату линии.
	/// ¬озвращает ближайшую точку на линии к заданной точке.
	/// </summary>
	Vector2 ClosestPointOnLine(const Vector2& p0, const Vector2& s1, const Vector2& s2);

	/// <summary>
	/// ѕринимает точку, начальную и конечную точку отрезка.
	/// ¬озвращает точку на отрезке, ближайшую к заданной точке.
	/// </summary>
	Vector2 ClosestPointOnLineSegment(const Vector2& p0, const Vector2& s1, const Vector2& s2);

	bool IsRectContainsPoint(const Vector2& r0, const Vector2& r1, const Vector2& r2, const Vector2& r3, const Vector2& p0);

};

