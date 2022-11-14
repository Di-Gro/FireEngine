#pragma once

#include <d3d11.h>
#include <SimpleMath.h>

using namespace DirectX::SimpleMath;

///
/// При добавлении в другой объект, нужно пересчитатьт локальную позицию так, 
/// чтобы мировая позиция не изменилась. 
///

class GameObject;

class Transform {
	friend class GameObject;

public:
	

private:

	Vector3 m_localPosition = Vector3::Zero;
	//Vector3 localPosition = Vector3::Zero;
	Vector3 m_localRotation = Vector3::Zero;
	Vector3 m_scale = Vector3::One;

	Matrix m_localMatrix;

	GameObject* friend_gameObject;
	//bool friend_dirty = true;

	Matrix m_matrix = Matrix::Identity;

public:
	Matrix GetWorldMatrix();
	Matrix GetLocalMatrix();

	const Vector3 localPosition();
	const Vector3 localRotation();
	const Quaternion localRotationQ();
	const Vector3 localScale();

	const Vector3 localForward();
	const Vector3 localUp();
	const Vector3 localRight();

	const Vector3 forward();
	const Vector3 up();
	const Vector3 right();

	void localPosition(const Vector3& value);
	void localRotation(const Vector3& value);
	void localRotation(const Quaternion& value);
	void localRotation(const Vector3& axis, float angle);
	void localScale(const Vector3& value);

	Vector3 worldPosition();
	Vector3 worldRotation();
	Matrix tmp_GetMatrixRL();
	Vector3 worldScale();
	//void worldPosition(Vector3 value);

private:
	Transform() = default;

	void friend_ChangeParent(GameObject* newParent);

};