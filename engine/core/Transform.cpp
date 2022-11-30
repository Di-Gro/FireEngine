#include "transform.h"

#include "Game.h"
#include "Math.h"

#include "Refs.h"




const Vector3 Transform::localPosition() { 
	return m_localPosition; 
}

void Transform::localPosition(const Vector3& value) { 
	m_localPosition = value;
	m_localMatrix.Translation(m_localPosition);
}

const Vector3 Transform::localRotation() { 
	return m_localRotation; 
}

const Quaternion Transform::localRotationQ() {
	//return Quaternion::CreateFromRotationMatrix(m_localMatrix);
	return Quaternion::CreateFromRotationMatrix(m_localRotationMatrix);
}

inline float ClampAngle(const float& angle) {
	float d = deg(angle);
	if (d >= 0 && d <= 360)
		return angle;

	float c = d - ((int)(d / 360) * 360);
	if (d > 360) 
		return rad(c);
	return rad(360 + c);
}

void Transform::localRotation(const Vector3& value) {
	m_localRotation.x = ClampAngle(value.x);
	m_localRotation.y = ClampAngle(value.y);
	m_localRotation.z = ClampAngle(value.z);

	auto q = Quaternion::CreateFromYawPitchRoll(m_localRotation);
	m_localRotationMatrix = Matrix::CreateFromQuaternion(q);

	m_localMatrix = Matrix::CreateScale(m_scale);
	m_localMatrix *= m_localRotationMatrix; // Matrix::CreateFromQuaternion(q);
	m_localMatrix *= Matrix::CreateTranslation(m_localPosition);
}

void Transform::localRotationQ(const Quaternion& value) {
	m_localRotation = value.ToEuler();
	m_localRotationMatrix = Matrix::CreateFromQuaternion(value);

	m_localMatrix = Matrix::CreateScale(m_scale);
	m_localMatrix *= m_localRotationMatrix; // Matrix::CreateFromQuaternion(value);
	m_localMatrix *= Matrix::CreateTranslation(m_localPosition);
}

void Transform::localRotation(const Vector3& axis, float angle) {
	auto q = Quaternion::CreateFromAxisAngle(axis, angle);
	m_localRotation = q.ToEuler();
	m_localRotationMatrix = Matrix::CreateFromQuaternion(q);

	m_localMatrix = Matrix::CreateScale(m_scale);
	m_localMatrix *= m_localRotationMatrix; // Matrix::CreateFromQuaternion(q);
	m_localMatrix *= Matrix::CreateTranslation(m_localPosition);
}

void Transform::localScale(const Vector3& value) {
	m_scale = value;

	//auto q = Quaternion::CreateFromYawPitchRoll(m_localRotation);
	auto q = Quaternion::CreateFromRotationMatrix(m_localRotationMatrix);

	m_localMatrix = Matrix::CreateScale(m_scale);
	m_localMatrix *= m_localRotationMatrix; // Matrix::CreateFromQuaternion(q);
	m_localMatrix *= Matrix::CreateTranslation(m_localPosition);
}

const Vector3 Transform::localScale() { return m_scale; }

const Vector3 Transform::localForward() { return m_localMatrix.Forward(); }
const Vector3 Transform::localUp()		{ return m_localMatrix.Up(); }
const Vector3 Transform::localRight()	{ return m_localMatrix.Right(); }

const Vector3 Transform::forward()	{ return GetWorldMatrix().Forward(); }
const Vector3 Transform::up()		{ return GetWorldMatrix().Right(); }
const Vector3 Transform::right()	{ return GetWorldMatrix().Right(); }

Matrix Transform::GetWorldMatrix() {
	auto matrix = GetLocalMatrix();

	auto parent = friend_gameObject->GetParent();

	//if (parent == nullptr)
	//	matrix *= Matrix::CreateScale({ 2, 2, 2 });

	while (parent != nullptr) {
		matrix *= parent->transform->GetLocalMatrix();
		parent = parent->GetParent();
	}
	m_matrix = matrix;
	return m_matrix;
}

Matrix Transform::GetLocalMatrix() {
	return m_localMatrix;
}

Vector3 Transform::worldPosition() {
	return GetWorldMatrix().Translation();
}

Matrix Transform::tmp_GetMatrixRL() {
	auto parentsWorldMatrix = Matrix::Identity;

	if (friend_gameObject->HasParent())
		parentsWorldMatrix = friend_gameObject->GetParent()->transform->GetWorldMatrix();

	return parentsWorldMatrix * GetLocalMatrix();
}

Vector3 Transform::worldRotation() {
	return tmp_GetMatrixRL().ToEuler();
}

Vector3 Transform::worldScale() {
	Quaternion quat;
	Vector3 nScale;
	Vector3 nPos;
	GetWorldMatrix().Decompose(nScale, quat, nPos);
	return nScale;
}

void PrintMatrix(Matrix m) {
	std::cout << m._11 << " " << m._12 << " " << m._13 << " " << m._14 << "\n";
	std::cout << m._21 << " " << m._22 << " " << m._23 << " " << m._24 << "\n";
	std::cout << m._31 << " " << m._32 << " " << m._33 << " " << m._34 << "\n";
	std::cout << m._41 << " " << m._42 << " " << m._43 << " " << m._44 << "\n";
	std::cout << "\n";
}

void Transform::friend_ChangeParent(GameObject* newParent) {
	auto m = GetWorldMatrix();

	if (newParent != nullptr) {
		m *= newParent->transform->GetWorldMatrix().Invert();
	}
	
	m_localMatrix = m;

	Quaternion quat;
	Vector3 nScale;
	Vector3 nPos;
	bool res = m.Decompose(nScale, quat, nPos);

	m_localPosition = m_localMatrix.Translation();
	m_localRotation = quat.ToEuler();
	m_localRotationMatrix = Matrix::CreateFromQuaternion(quat);
}

#pragma warning( push )
#pragma warning( disable : 4190)

DEF_OBJECT(Transform, 0) { }

DEF_PROP_GETSET(Transform, Vector3, localPosition)
DEF_PROP_GETSET(Transform, Vector3, localRotation)
DEF_PROP_GETSET(Transform, Quaternion, localRotationQ)
DEF_PROP_GETSET(Transform, Vector3, localScale)

DEF_PROP_GET(Transform, Vector3, localForward)
DEF_PROP_GET(Transform, Vector3, localUp)
DEF_PROP_GET(Transform, Vector3, localRight)

DEF_PROP_GET(Transform, Vector3, forward)
DEF_PROP_GET(Transform, Vector3, up)
DEF_PROP_GET(Transform, Vector3, right)


//Vector3 Transform_localPosition_get(CppRef objRef) {
//	auto* a = CppRefs::GetPointer<Transform>(objRef);
//	if (a != nullptr)
//		return a->localPosition();
//	else
//		std::cout << "+: prop_get::GetPointer<>(" << objRef << "): NULL" << std::endl;
//
//	return Vector3();
//}
//
//void Transform_localPosition_set(CppRef objRef, Vector3 value) {		
//	auto* a = CppRefs::GetPointer<Transform>(objRef);
//	if (a != nullptr)
//		a->localPosition(value);
//	else
//		std::cout << "+: prop_set::GetPointer<>(" << objRef << "): NULL" << std::endl;
//
//}

#pragma warning( pop ) 
