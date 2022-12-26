#pragma once

#include "Actor.h"

class Player;
class MeshComponent;
class Material;

class Attachable : public Component {
	PURE_COMPONENT(Attachable)

private:
	static Material* s_meshMaterial;
	static Material* s_boundMaterial;
	static size_t s_attachableCount;

public:
	Player* player = nullptr;

	float boundRadius = 30;
	bool attachParent = false;

	bool showCenter = true;
	bool showBound = true;

	Vector3 boundCenter;
	
private:
	MeshComponent* m_mesh;
	MeshComponent* m_bound;

	bool m_canAttach = false;

public:
	//Attachable(Actor* actor) : Component(actor) { }

	void OnInit() override;
	void OnStart() override;
	void OnUpdate() override;
	void OnDestroy() override;
	
};

