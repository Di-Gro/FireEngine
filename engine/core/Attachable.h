#pragma once

#include "Game.h"

class Player;
class MeshComponent;

class Attachable : public Component {
	PURE_COMPONENT;
public:
	Player* player = nullptr;

	float boundRadius = 30;
	bool attachParent = false;

	bool showCenter = true;
	bool showBound = true;
	
private:
	MeshComponent* m_mesh;
	MeshComponent* m_bound;

	bool m_canAttach = false;

public:
	//Attachable(GameObject* gameObject) : Component(gameObject) { }

	void OnInit() override;
	void OnStart() override;
	void OnUpdate() override;
	
};

