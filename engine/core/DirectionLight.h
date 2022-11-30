#pragma once

#include "Game.h"
#include "RenderTarget.h"

class MeshComponent;
class LineComponent;
class CameraComponent;

class DirectionLight : public Component {
	PURE_COMPONENT;
public:
	Vector3 color = Vector3::One;
	float intensity = 1.0f;

	RenderTarget shadowRT;

private:

	CameraComponent* m_camera = nullptr;

	MeshComponent* m_debugMesh = nullptr;
	LineComponent* m_debugLine = nullptr;

	Matrix m_uvMatrix;

	bool m_needDrawDebug = true;

public:
	//DirectionLight(GameObject* gameObject) : Component(gameObject) { }

	bool drawDebug() { return m_needDrawDebug; }
	void drawDebug(bool value);

	CameraComponent* camera();
	Matrix uvMatrix() { return m_uvMatrix; }

	void OnInit() override;

	void RecieveGameMessage(const std::string& msg);
};

