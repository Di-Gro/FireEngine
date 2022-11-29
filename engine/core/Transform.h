#pragma once

#include <d3d11.h>
#include <SimpleMath.h>

#include "ClassInfo.h"
#include "CSBridge.h"
#include "CsLink.h"

using namespace DirectX::SimpleMath;

///
/// При добавлении в другой объект, нужно пересчитатьт локальную позицию так, 
/// чтобы мировая позиция не изменилась. 
///

class Game;
class GameObject;
class Component;


class Transform : public CsLink {
	OBJECT;

	friend class GameObject;

private:
	Vector3 m_localPosition = Vector3::Zero;
	Vector3 m_localRotation = Vector3::Zero;
	Vector3 m_scale = Vector3::One;

	Matrix m_localMatrix;
	Matrix m_localRotationMatrix;

	GameObject* friend_gameObject;

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
	void localRotationQ(const Quaternion& value);
	void localRotation(const Vector3& axis, float angle);
	void localScale(const Vector3& value);

	Vector3 worldPosition();
	Vector3 worldRotation();
	Matrix tmp_GetMatrixRL();
	Vector3 worldScale();

private:
	Transform() = default; 

	void friend_ChangeParent(GameObject* newParent);
};

#pragma warning( push )
#pragma warning( disable : 4190)

PROP_GETSET(Transform, Vector3, localPosition)
PROP_GETSET(Transform, Vector3, localRotation)
PROP_GETSET(Transform, Quaternion, localRotationQ)
PROP_GETSET(Transform, Vector3, localScale)

PROP_GET(Transform, Vector3, localForward)
PROP_GET(Transform, Vector3, localUp)
PROP_GET(Transform, Vector3, localRight)

PROP_GET(Transform, Vector3, forward)
PROP_GET(Transform, Vector3, up)
PROP_GET(Transform, Vector3, right)


#pragma warning( pop ) 