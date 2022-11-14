#pragma once

#include "SimpleMath.h"

#include "Window.h"
#include "Math.h"

#include "Game.h"

class Shader;

class CameraComponent : public Component {
public:
	float orthoWidth = 600;
	float orthoHeight = 600;
	float orthoNearPlane = 50;
	float orthoFarPlane = 1000;

	float nearPlane = 0.1f;
	float farPlane = 10'000.0f;

	bool drawDebug = true;
	bool callPixelShader = true;
	bool setShaderMap = true;
	//Shader* shader = nullptr;

private:

	Matrix m_viewMatrix = Matrix::Identity;
	Matrix m_projMatrix;

	bool m_useOrthographic = false;

public:

	CameraComponent(GameObject* gameObject) : Component(gameObject) {}

	Matrix cameraMatrix() { return m_viewMatrix * m_projMatrix; }

	const Matrix& viewMatrix() { return m_viewMatrix; }
	void viewMatrix(const Matrix& value) { m_viewMatrix = value; }

	const Matrix& projMatrix() { return m_projMatrix; }

	void orthographic(bool value) { m_useOrthographic = value; }
	bool orthographic() { return m_useOrthographic; }

	void Attach();
	bool IsAttached();

	void UpdateProjectionMatrix(Window* window);

};

//Vector3 GetForward() {
//	Matrix invert;
//	m_viewMatrix.Invert(invert);
//	return invert.Forward();
//}

//Vector3 GetUp() {
//	Matrix invert;
//	m_viewMatrix.Invert(invert);
//	return invert.Up();
//}

//Vector3 GetRight() {
//	Matrix invert;
//	m_viewMatrix.Invert(invert);
//	return invert.Right();
//}

//Vector3 GetPosition() {
//	//return -m_viewMatrix.Translation();
//	Matrix invert;
//	m_viewMatrix.Invert(invert);
//	return invert.Translation(); 
//}

