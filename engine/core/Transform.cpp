#include "transform.h"

#include "Game.h"
#include "Actor.h"
#include "Math.h"
#include "SimpleMath.h"

#include "Refs.h"

const Vector3 Transform::localPosition() { 
	return m_localPosition; 
}

void Transform::SetLocalMatrix(Matrix matrix) {
	Quaternion quat = Quaternion::Identity;
	Vector3 nScale = Vector3::Zero;
	Vector3 nPos = Vector3::Zero;

	bool res = matrix.Decompose(nScale, quat, nPos);

	m_localMatrix = matrix;
	m_localPosition = m_localMatrix.Translation();
	m_localRotation = quat.ToEuler();
	m_localRotationMatrix = Matrix::CreateFromQuaternion(quat);
	m_localScale = nScale;
}

void Transform::localPosition(const Vector3& value) { 
	m_localPosition = value;
	m_localMatrix.Translation(m_localPosition);
}

void Transform::worldPosition(const Vector3& value) {
	localPosition(m_localPosition + value - worldPosition());
}

void Transform::worldRotationQ(const Quaternion& value) {
	Quaternion iwrot;
	worldRotationQ().Inverse(iwrot);
	localRotationQ(localRotationQ() * (value * iwrot));
}

void Transform::worldScale(const Vector3& value) {
	auto parentWorldScale = Vector3::One; // parentWorldScale
	auto newLocalScale = Vector3::Zero;

	if (friend_gameObject->HasParent())
		parentWorldScale = friend_gameObject->parent()->worldScale();

	if (parentWorldScale.x != 0) newLocalScale.x = value.x / parentWorldScale.x;
	if (parentWorldScale.y != 0) newLocalScale.y = value.y / parentWorldScale.y;
	if (parentWorldScale.z != 0) newLocalScale.z = value.z / parentWorldScale.z;

	localScale(newLocalScale);
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

	m_localMatrix = Matrix::CreateScale(m_localScale);
	m_localMatrix *= m_localRotationMatrix;
	m_localMatrix *= Matrix::CreateTranslation(m_localPosition);
}

void Transform::localRotationQ(const Quaternion& value) {
	m_localRotation = value.ToEuler();
	m_localRotationMatrix = Matrix::CreateFromQuaternion(value);

	m_localMatrix = Matrix::CreateScale(m_localScale);
	m_localMatrix *= m_localRotationMatrix;
	m_localMatrix *= Matrix::CreateTranslation(m_localPosition);
}

void Transform::localRotation(const Vector3& axis, float angle) {
	auto q = Quaternion::CreateFromAxisAngle(axis, angle);
	m_localRotation = q.ToEuler();
	m_localRotationMatrix = Matrix::CreateFromQuaternion(q);

	m_localMatrix = Matrix::CreateScale(m_localScale);
	m_localMatrix *= m_localRotationMatrix;
	m_localMatrix *= Matrix::CreateTranslation(m_localPosition);
}

void Transform::localScale(const Vector3& value) {
	m_localScale = value;

	auto q = Quaternion::CreateFromRotationMatrix(m_localRotationMatrix);

	m_localMatrix = Matrix::CreateScale(m_localScale);
	m_localMatrix *= m_localRotationMatrix;
	m_localMatrix *= Matrix::CreateTranslation(m_localPosition);
}

const Vector3 Transform::localScale() { return m_localScale; }

const Vector3 Transform::localForward() { return m_localMatrix.Forward().Normalized(); }
const Vector3 Transform::localUp()		{ return m_localMatrix.Up().Normalized(); }
const Vector3 Transform::localRight()	{ return m_localMatrix.Right().Normalized(); }

const Vector3 Transform::forward()	{ return GetWorldMatrix().Forward().Normalized(); }
const Vector3 Transform::up()		{ return GetWorldMatrix().Up().Normalized(); }
const Vector3 Transform::right()	{ return GetWorldMatrix().Right().Normalized(); }

Matrix Transform::GetWorldMatrix() {
	auto matrix = GetLocalMatrix();
	auto parent = friend_gameObject->parent();

	while (parent != nullptr) {
		matrix *= parent->GetLocalMatrix();
		parent = parent->parent();
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
		parentsWorldMatrix = friend_gameObject->parent()->GetWorldMatrix();

	return parentsWorldMatrix * GetLocalMatrix();
}

Vector3 Transform::worldRotation() {
	return tmp_GetMatrixRL().ToEuler();
}

Quaternion Transform::worldRotationQ() {
	auto wmat = GetWorldMatrix();

	Quaternion quat;
	Vector3 nScale;
	Vector3 nPos;
	bool res = wmat.Decompose(nScale, quat, nPos);

	return quat;
}

Vector3 Transform::worldScale() {
	auto scale = localScale();
	auto parent = friend_gameObject->parent();

	while (parent != nullptr) {
		scale *= parent->localScale();
		parent = parent->parent();
	}
	return scale;

	//Quaternion quat;
	//Vector3 nScale;
	//Vector3 nPos;
	//GetWorldMatrix().Decompose(nScale, quat, nPos);
	//return nScale;
}

void PrintMatrix(Matrix m) {
	std::cout << m._11 << " " << m._12 << " " << m._13 << " " << m._14 << "\n";
	std::cout << m._21 << " " << m._22 << " " << m._23 << " " << m._24 << "\n";
	std::cout << m._31 << " " << m._32 << " " << m._33 << " " << m._34 << "\n";
	std::cout << m._41 << " " << m._42 << " " << m._43 << " " << m._44 << "\n";
	std::cout << "\n";
}

void Transform::friend_ChangeParent(Actor* newParent) {
	auto m = GetWorldMatrix();

	if (newParent != nullptr)
		m *= newParent->GetWorldMatrix().Invert();
	
	Quaternion quat = Quaternion::Identity;
	Vector3 nScale = Vector3::Zero;
	Vector3 nPos = Vector3::Zero;

	if (IsNaN(m)) 
		FixNaN(m);
	else 
		bool res = m.Decompose(nScale, quat, nPos);

	m_localMatrix = m;
	m_localPosition = m_localMatrix.Translation();
	m_localRotation = quat.ToEuler();
	m_localRotationMatrix = Matrix::CreateFromQuaternion(quat);
	m_localScale = nScale;
}

//#pragma warning( push )
//#pragma warning( disable : 4190)
//
//DEF_OBJECT(Transform, 0) { }
//
//
//
//
//#pragma warning( pop ) 
