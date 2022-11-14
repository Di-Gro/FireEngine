#pragma once

#include "Game.h"
#include "MeshComponent.h"
#include "Attachable.h"

class PlayerCamera;
class BoxCollider;

class Player : public Component {
private:
	PlayerCamera* m_playerCamera = nullptr;
	GameObject* m_playerMesh = nullptr;
	GameObject* m_playerBound = nullptr;

	float m_rotationSpeed = 0;
	float m_startRadius = 0;
	float m_startSpeed = 0;

	bool m_onCollision = false;
	Vector3 m_hitPoint;
	BoxCollider* m_hitCollider;
	
	Vector3 m_velocityDelta;
	Vector3 m_lastVelocity;
	Quaternion m_rotationDelta;

	bool m_updatePosition = false;
	bool m_updateRotation = false;

	GameObject* m_velPoint = nullptr;

public:
	float radius = 25;
	float speed = 100;

	bool showAxis = false;

public:
	Player(GameObject* gameObject) : Component(gameObject) { }

	void OnInit() override;
	void OnStart() override;
	void OnUpdate() override;

	void Move(Vector3 direction);


	void Attach(GameObject* attachableObj);
	void OnCollisionBegin(BoxCollider* collider);
	void OnCollision(BoxCollider* collider);
	void OnCollisionEnd(BoxCollider* collider);

};

