#pragma once

#include "Game.h"

class Player;
class MeshComponent;

class Attachable : public Component {
	PURE_COMPONENT(Attachable)
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

	Material* m_meshMaterial;
	Material* m_boundMaterial;

	bool m_canAttach = false;

public:
	//Attachable(Actor* actor) : Component(actor) { }

	void OnInit() override;
	void OnStart() override;
	void OnUpdate() override;
	void OnDestroy() override;
	
};

