#pragma once

#include <d3d11.h>
#include <SimpleMath.h>

using namespace DirectX::SimpleMath;


namespace LineMath {

	/// <summary>
	/// ���� ����� ����������� �������� ������.
	/// ���������� true ��� false, ���� ����������� ���� ��� ���, ��������������.
	/// </summary>
	bool Intersection(
		const Vector2& p11,
		const Vector2& p12,
		const Vector2& p21,
		const Vector2& p22,
		Vector2& intersect);

	/// <summary>
	/// ���� ����� ����������� ����� �������� ������ � ��������.
	/// ���������� true ��� false, ���� ����������� ���� ��� ���, ��������������.
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
	/// ���������� ��������� �� ����� � �������� �������, � �.�. ������ ��� �����.
	/// </summary>
	bool IsClamped(
		const Vector2& p,
		const Vector2& s1,
		const Vector2& s2);

	/// <summary> 
	/// ���������� ������������ �� �������� �������.
	/// </summary>
	bool IsIntersectedSegments(
		const Vector2& p1,
		const Vector2& p2,
		const Vector2& m1,
		const Vector2& m2);

	/// <summary>
	///  ��������� �����, ��������� � �������� ���������� �����.
	///  ���������� ����� ������������ ����� ��������� ������� � ������.
	///  result < 0 - ����� ����� ��� ������ (������)
	///  result > 0 - ����� ����� ��� ������ (�����)
	///  result = 0 - ����� ����� �� �����
	/// </summary>
	float PseudoDotProduct(
		const Vector2& p0,
		const Vector2& p1,
		const Vector2& p2);

	float Angle( const Vector2& v1, const Vector2& v2);
	float SignedAngle(const Vector2& v1, const Vector2& v2);

	Vector2 Rotate(const Vector2& v, float angle);

	/// <summary>
	/// ��������� �����, ��������� � �������� ���������� �����.
	/// ���������� ��������� ����� �� ����� � �������� �����.
	/// </summary>
	Vector2 ClosestPointOnLine(const Vector2& p0, const Vector2& s1, const Vector2& s2);

	/// <summary>
	/// ��������� �����, ��������� � �������� ����� �������.
	/// ���������� ����� �� �������, ��������� � �������� �����.
	/// </summary>
	Vector2 ClosestPointOnLineSegment(const Vector2& p0, const Vector2& s1, const Vector2& s2);

	bool IsRectContainsPoint(const Vector2& r0, const Vector2& r1, const Vector2& r2, const Vector2& r3, const Vector2& p0);

};

