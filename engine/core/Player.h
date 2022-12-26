#pragma once

#include "Actor.h"

class PlayerCamera;
class BoxCollider;
class Mesh4;
class Material;
class MeshComponent;

class Player : public Component {
	PURE_COMPONENT(Player)
private:
	PlayerCamera* m_playerCamera = nullptr;
	Actor* m_playerMesh = nullptr;
	Actor* m_playerBound = nullptr;

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

	Actor* m_velPoint = nullptr;

	MeshComponent* m_boxMesh;
	MeshComponent* m_boundSphere;

	Material* m_boxMeshMaterial;
	Material* m_boundSphereMaterial;

public:
	float radius = 25;
	float speed = 100;

	bool showAxis = false;

public:
	void OnInit() override;
	void OnStart() override;
	void OnUpdate() override;
	void OnDestroy() override;

	void Move(Vector3 direction);

	void Attach(Actor* attachableObj);
	void OnCollisionBegin(BoxCollider* collider);
	void OnCollision(BoxCollider* collider);
	void OnCollisionEnd(BoxCollider* collider);

};

