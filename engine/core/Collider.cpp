//#include "Collider.h"
//
//#include "Game.h"
//
//#include "SimpleMath.h"
//#include "Player.h"
//#include "MeshComponent.h"
//#include "Forms.h"
//#include "LineMath.h"
//
//#include <DirectXCollision.h>
//
//using XRect = DirectX::SimpleMath::Rectangle;
//
//void BoxCollider::OnStart() { }
//
//void BoxCollider::OnUpdate() {
//	if (player != nullptr && player->IsDestroyed())
//		player = nullptr;
//
//	if (player == nullptr)
//		return;
//
//	if (Intersects()) {
//		if (!m_hasIntersection) {
//			m_hasIntersection = true;
//			player->OnCollisionBegin(this);
//		}
//		else {
//			player->OnCollision(this);
//		}
//	}
//	else if(m_hasIntersection) {
//		m_hasIntersection = false;
//		player->OnCollisionEnd(this);
//	}
//}
//
//bool BoxCollider::Intersects() {
//	auto worldPos = player->worldPosition();
//	auto scale = localScale();
//	auto extends = boundSize / 2 * scale;
//
//	auto box = DirectX::BoundingOrientedBox(worldPosition(), extends, localRotationQ());
//	auto sphere = DirectX::BoundingSphere(worldPos, player->radius);
//	
//	return box.Intersects(sphere);
//
//	//auto boxSize = boundSize + Vector3::One.Normalized() * (player->radius * 2);
//	//auto ltpos = worldPosition() - boxSize / 2;
//
//	//auto boxWorld = Matrix::CreateWorld(ltpos, forward(), up());
//	//auto boxInvert = boxWorld.Invert();
//
//	//auto p = (Matrix::CreateTranslation(worldPos) * boxInvert).Translation();
//
//	//if (p.x < 0 || p.x > boxSize.x ||
//	//	p.y < 0 || p.y > boxSize.y ||
//	//	p.z < 0 || p.z > boxSize.z)
//	//{
//	//	return false;
//	//}
//	////std::cout << "Collide " << p.x << " " << p.y << " " << p.z << "\n";
//	//return true;
//}
//
//bool BoxCollider::FloorContainPoint(Vector3 point) {
//	auto worldPos = point; // player->worldPosition();
//	auto scale = localScale();
//	auto extends = boundSize / 2 * scale;
//
//	auto box = DirectX::BoundingOrientedBox(worldPosition(), extends, localRotationQ());
//
//	DirectX::XMFLOAT3 corners[8] = {};
//	box.GetCorners(corners);
//
//	int plainInds[18] = {
//		0,1,2, 0,3,4, 0,1,4,
//		6,2,5, 6,2,7, 6,5,4,
//	};
//
//	auto r0 = Vector2(corners[0].x, corners[0].z);
//	auto r1 = Vector2(corners[4].x, corners[4].z);
//	auto r2 = Vector2(corners[5].x, corners[5].z);
//	auto r3 = Vector2(corners[1].x, corners[1].z);
//	auto p0 = Vector2(point.x, point.z);
//
//	return LineMath::IsRectContainsPoint(r0, r1, r2, r3, p0);
//}
//
//Vector2 BoxCollider::GetClosestFloorPoint(Vector3 point) {
//	auto worldPos = point;
//	auto scale = localScale();
//	auto extends = boundSize / 2 * scale;
//
//	auto box = DirectX::BoundingOrientedBox(worldPosition(), extends, localRotationQ());
//
//	DirectX::XMFLOAT3 corners[8] = {};
//	box.GetCorners(corners);
//
//	int plainInds[18] = {
//		0,1,2, 0,3,4, 0,1,4,
//		6,2,5, 6,2,7, 6,5,4,
//	};
//
//	auto r0 = Vector2(corners[0].x, corners[0].z);
//	auto r1 = Vector2(corners[4].x, corners[4].z);
//	auto r2 = Vector2(corners[5].x, corners[5].z);
//	auto r3 = Vector2(corners[1].x, corners[1].z);
//	auto p0 = Vector2(point.x, point.z);
//
//	Vector2 lines[] = {
//		r0, r1,
//		r1, r2, 
//		r2, r3,
//		r3, r0,
//	};
//
//	float minDist = (std::numeric_limits<float>::max)();
//	Vector2 minPoint;
//
//	for (int i = 0; i < 8; i += 2) {
//		auto closest = LineMath::ClosestPointOnLineSegment(p0, lines[i], lines[i + 1]);
//
//		float dist = (closest - p0).Length();
//		if (dist < minDist) {
//			minDist = dist;
//			minPoint = closest;
//		}
//	}
//
//	return minPoint;
//}
//
//bool BoxCollider::GetIntersectPoint(Vector3 lineBegin, Vector3 lineEnd) {
//	auto worldPos = player->worldPosition();
//	auto scale = localScale();
//	auto extends = boundSize / 2 * scale;
//
//	auto box = DirectX::BoundingOrientedBox(worldPosition(), extends, localRotationQ());
//
//	DirectX::XMFLOAT3 corners[8] = {};
//	box.GetCorners(corners);
//
//	int plainInds[18] = {
//		0,1,2, 0,3,4, 0,1,4,
//		6,2,5, 6,2,7, 6,5,4,
//	};
//
//	float minDist = (std::numeric_limits<float>::max)();
//	Vector3 minPoint;
//
//	for (int i = 0; i < 18; i += 3) {
//		auto point0 = DirectX::XMLoadFloat3(&corners[plainInds[i + 0]]);
//		auto point1 = DirectX::XMLoadFloat3(&corners[plainInds[i + 1]]);
//		auto point2 = DirectX::XMLoadFloat3(&corners[plainInds[i + 2]]);
//
//		auto plain = DirectX::XMPlaneFromPoints(point0, point1, point2);
//		
//		DirectX::SimpleMath::Plane pl = plain;
//		DirectX::SimpleMath::Rectangle rect();
//
//		auto intersect = (Vector3)DirectX::XMPlaneIntersectLine(plain, lineBegin, lineEnd);
//		
//		if (!box.Contains(intersect) || intersect == Vector3::Zero)
//			continue;
//
//		float dist = (intersect - lineBegin).Length();
//		if (dist < minDist) {
//			minDist = dist;
//			minPoint = intersect;
//		}
//	}
//
//	if (minPoint == Vector3::Zero)
//		return false;
//	
//	auto mesh = CreateActor("hit")->AddComponent<MeshComponent>();
//	auto form = Forms4::Box({ 5,5,5 }, { 1,0,0,1 });
//	mesh->AddShape(&form.verteces, &form.indexes);
//	mesh->localPosition(minPoint);
//
//	std::cout << minPoint.x << " " << minPoint.y << " " << minPoint.z << "\n";
//
//	return true;
//}