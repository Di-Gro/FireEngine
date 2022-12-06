#include "Player.h"
#include "PlayerCamera.h"
#include "MeshComponent.h"
#include "LineComponent.h"
#include "Forms.h"
#include "Collider.h"

DEF_PURE_COMPONENT(Player);

void Player::OnInit() {
	m_rotationSpeed = rad(speed / (2 * pi * radius) * 360);
	m_startRadius = radius;
	m_startSpeed = speed;

	m_playerMesh = CreateActor("player mesh");
	m_playerBound = CreateActor("player bound");

	m_playerMesh->parent(this);
	m_playerBound->parent(this);

	auto meshAsset = game()->meshAsset();

	m_boxMeshMaterial = meshAsset->CreateDynamicMaterial("boxMeshMaterial", MeshAsset::defaultShader);
	m_boxMeshMaterial->data.diffuseColor = { 0.8, 0.6, 0.2, 1.0 };
	
	m_boundSphereMaterial = meshAsset->CreateDynamicMaterial("boundSphereMaterial", "../../data/engine/shaders/defuse_color.hlsl");
	m_boundSphereMaterial->data.diffuseColor = { 0, 0.6, 0, 1 };

	float boxSize = radius * 2 * 0.9;
	m_boxMesh = m_playerMesh->AddComponent<MeshComponent>();
	m_boxMesh->localScale({ boxSize, boxSize, boxSize });
	m_boxMesh->mesh(meshAsset->GetMesh(MeshAsset::formBox));
	m_boxMesh->SetMaterial(0, m_boxMeshMaterial);

	m_boundSphere = m_playerBound->AddComponent<MeshComponent>();
	m_boundSphere->localScale({ radius * 2, radius * 2, radius * 2 });
	m_boundSphere->mesh(meshAsset->GetMesh(MeshAsset::formSphereLined));
	m_boundSphere->SetMaterial(0, m_boundSphereMaterial);
	m_boundSphere->isDebug = true;

	if (showAxis) {
		AddComponent<LineComponent>()->SetPoint(Vector3::Forward * 100, { 1, 1, 1, 1 });
		AddComponent<LineComponent>()->SetPoint(Vector3::Up * 100, { 0, 1, 0, 1 });
		AddComponent<LineComponent>()->SetPoint(Vector3::Right * 100, { 1, 0, 0, 1 });

		m_playerMesh->AddComponent<LineComponent>()->SetPoint(Vector3::Forward * 60, { 1, 1, 1, 1 });
		m_playerMesh->AddComponent<LineComponent>()->SetPoint(Vector3::Up * 60, { 0, 1, 0, 1 });
		m_playerMesh->AddComponent<LineComponent>()->SetPoint(Vector3::Right * 60, { 1, 0, 0, 1 });
	}

	auto form = Forms4::Box({ 5,5,5 }, { 1,0,0,1 });
	auto mesh = CreateActor("vel point")->AddComponent<MeshComponent>();
	mesh->AddShape(&form.verteces, &form.indexes, 0);
	mesh->localPosition(worldPosition());

	m_velPoint = mesh->actor();
}

void Player::OnDestroy() {
	game()->meshAsset()->DeleteDynamicMaterial(m_boxMeshMaterial);
	game()->meshAsset()->DeleteDynamicMaterial(m_boundSphereMaterial);
}

void Player::OnStart() {
	m_playerCamera = GetComponentInChild<PlayerCamera>();
	if (!m_playerCamera)
		std::cout << "Player haven't got a PlayerCamera component\n";

	auto pos = localPosition();
	pos.y = radius;
	localPosition(pos);
}

void Player::OnUpdate() {
	if (m_updatePosition) {

		if (m_onCollision) {
			auto nextPoint = m_hitPoint + m_velocityDelta;
			if (m_hitCollider->FloorContainPoint(nextPoint)) {
				auto xz = m_hitCollider->GetClosestFloorPoint(nextPoint);
				Vector3 closest = { xz.x, m_hitPoint.y, xz.y };

				m_velocityDelta = closest - m_hitPoint;
			}
		}
		localPosition(localPosition() + m_velocityDelta);
		m_lastVelocity = m_velocityDelta;
		m_velocityDelta = Vector3::Zero;
		m_updatePosition = false;
	}
	if (m_updateRotation) {
		auto rot = m_playerMesh->localRotationQ();
		rot *= m_rotationDelta;

		m_playerMesh->localRotationQ(rot);
		m_playerBound->localRotationQ(rot);

		m_rotationDelta = Quaternion();
		m_updateRotation = false;
	}

	auto point = worldPosition();

	if (m_lastVelocity != Vector3::Zero)
		point += m_lastVelocity.Normalized() * radius + m_lastVelocity;

	m_velPoint->localPosition(point);
}

void Player::Move(Vector3 axis) {

	auto forward = m_playerCamera->forward();
	forward.y = 0;
	forward.Normalize();

	auto direction = forward * axis.x + m_playerCamera->right() * axis.z;

	if (direction != Vector3::Zero) {
		auto cross = direction.Cross(Vector3::Up);
		float angleDelta = m_rotationSpeed * direction.Length() * game()->deltaTime();

		m_velocityDelta += direction * speed * game()->deltaTime();
		m_rotationDelta *= Quaternion::CreateFromAxisAngle(-cross, angleDelta);

		m_updatePosition = true;
		m_updateRotation = true;
	}
}

void Player::Attach(Actor* attachableObj) {
	attachableObj->parent(m_playerMesh);

	auto attachable = attachableObj->GetComponentInChild<Attachable>();

	radius += attachable->boundRadius * 0.02f;
	float prog = radius / m_startRadius;
	speed = m_startSpeed * prog;

	auto scale = m_playerBound->localScale();
	scale = Vector3::One * m_startRadius * 2 * prog;
	m_playerBound->localScale(scale);

	auto pos = localPosition();
	pos.y = radius;
	localPosition(pos);
}

void Player::OnCollisionBegin(BoxCollider* collider) {
	static int index = 0;

	auto pos = worldPosition();
	auto xz = collider->GetClosestFloorPoint(pos);
	m_hitPoint = { xz.x, pos.y, xz.y };
	m_hitCollider = collider;

	m_onCollision = true;
	std::cout << "OnCollisionBegin " << index++ << "\n";
}

void Player::OnCollisionEnd(BoxCollider* collider) {
	static int index = 0;

	m_onCollision = false;
	m_hitCollider = nullptr;
	std::cout << "OnCollisionEnd " << index++ << "\n";
}

void Player::OnCollision(BoxCollider* collider) {
	auto pos = worldPosition();
	auto xz = collider->GetClosestFloorPoint(pos);
	m_hitPoint = { xz.x, pos.y, xz.y };
	m_hitCollider = collider;
}
