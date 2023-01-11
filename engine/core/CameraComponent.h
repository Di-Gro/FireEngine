#pragma once

#include <list>
#include "SimpleMath.h"

#include "Window.h"
#include "Math.h"

#include "Actor.h"
#include "Scene.h"

class Shader;

class CameraComponent : public Component {
	COMPONENT(CameraComponent)
public:
	bool isDefault = true;

	float orthoWidth = 600;
	float orthoHeight = 600;
	float orthoNearPlane = 50;
	float orthoFarPlane = 1000;

	float nearPlane = 0.1f;
	float farPlane = 10'000.0f;

	bool drawDebug = true;
	bool callPixelShader = true;
	bool setShaderMap = true;
	bool isPlayerCamera = false;

private:

	Matrix m_viewMatrix = Matrix::Identity;
	Matrix m_projMatrix;

	bool m_useOrthographic = false;

	CameraIter m_cameraIter;

public:

	Matrix cameraMatrix() { return m_viewMatrix * m_projMatrix; }

	const Matrix& viewMatrix() { return m_viewMatrix; }
	void viewMatrix(const Matrix& value) { m_viewMatrix = value; }

	const Matrix& projMatrix() { return m_projMatrix; }

	void orthographic(bool value) { m_useOrthographic = value; }
	bool orthographic() { return m_useOrthographic; }

	void Attach();
	void Deattach();
	bool IsAttached();
	bool IsMainCamera();

	void UpdateProjectionMatrix();

	void OnInit() override;
	void OnDestroy() override;

};
DEC_COMPONENT(CameraComponent);

PROP_GET(CameraComponent, bool, IsAttached)
PROP_GETSET(CameraComponent, bool, orthographic)
PROP_GETSET(CameraComponent, Matrix, viewMatrix)

FUNC(CameraComponent, Attach, void)(CppRef compRef);
FUNC(CameraComponent, UpdateProjMatrix, void)(CppRef compRef);