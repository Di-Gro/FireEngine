#pragma once

#include <d3d11.h>
#include <SimpleMath.h>

#include "ClassInfo.h"
#include "CSBridge.h"
#include "CsLink.h"

using namespace DirectX::SimpleMath;

class Game;
class Actor;
class Component;

class Transform /*: public CsLink*/ {
	OBJECT;

	friend class Actor;

private:
	Vector3 m_localPosition = Vector3::Zero;
	Vector3 m_localRotation = Vector3::Zero;
	Vector3 m_localScale = Vector3::One;

	Matrix m_localMatrix;
	Matrix m_localRotationMatrix;

	Actor* friend_gameObject;

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
	Quaternion worldRotationQ();
	Matrix tmp_GetMatrixRL();
	Vector3 worldScale();

	void worldPosition(const Vector3& value);
	void worldRotationQ(const Quaternion& value);
	void worldScale(const Vector3& value);

private:
	Transform() = default; 

	void friend_ChangeParent(Actor* newParent);
};

class ActorTransform {
	friend class Actor;

private:
	Transform* transform;

public:
	//CsRef transformCsRef() { return transform->csRef(); }
	//CppRef transformCppRef() { return transform->cppRef(); }

	inline Matrix GetWorldMatrix() { return transform->GetWorldMatrix(); }
	inline Matrix GetLocalMatrix() { return transform->GetLocalMatrix(); }

	inline const Vector3 localPosition() { return transform->localPosition(); }
	inline const Vector3 localRotation() { return transform->localRotation(); }
	inline const Quaternion localRotationQ() { return transform->localRotationQ(); }
	inline const Vector3 localScale() { return transform->localScale(); }

	inline const Vector3 localForward() { return transform->localForward(); }
	inline const Vector3 localUp() { return transform->localUp(); }
	inline const Vector3 localRight() { return transform->localRight(); }

	inline const Vector3 forward() { return transform->forward(); }
	inline const Vector3 up() { return transform->up(); }
	inline const Vector3 right() { return transform->right(); }

	inline void localPosition(const Vector3& value) { transform->localPosition(value); }
	inline void localRotation(const Vector3& value) { transform->localRotation(value); }
	inline void localRotationQ(const Quaternion& value) { transform->localRotationQ(value); }
	inline void localRotation(const Vector3& axis, float angle) { transform->localRotation(axis, angle); }
	inline void localScale(const Vector3& value) { transform->localScale(value); }

	inline Vector3 worldPosition() { return transform->worldPosition(); }
	inline Vector3 worldRotation() { return transform->worldRotation(); }
	inline Quaternion worldRotationQ() { return transform->worldRotationQ(); }
	inline Matrix tmp_GetMatrixRL() { return transform->tmp_GetMatrixRL(); }
	inline Vector3 worldScale() { return transform->worldScale(); }

	inline void worldPosition(const Vector3& value) { transform->worldPosition(value); }
	inline void worldRotationQ(const Quaternion& value) { transform->worldRotationQ(value); }
	inline void worldScale(const Vector3& value) { transform->worldScale(value); }
};
