//#pragma once
//
//#include "Actor.h"
//
//class Player;
//
//class BoxCollider : public Component {
//private:
//	bool m_hasIntersection = false;
//
//public:
//	Vector3 boundSize = Vector3::Zero;
//	Player* player = nullptr;
//
//public:
//	//BoxCollider(Actor* actor) : Component(actor) { }
//
//	void OnStart() override;
//	void OnUpdate() override;
//
//	bool Intersects();
//	bool FloorContainPoint(Vector3 lineBegin);
//	Vector2 GetClosestFloorPoint(Vector3 lineBegin);
//	bool GetIntersectPoint(Vector3 lineBegin, Vector3 lineEnd);
//};
//
