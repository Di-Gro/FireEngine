#pragma once

#include "Actor.h"

class Player;
class PlayerCamera;

class PlayerController : public Component {
	PURE_COMPONENT(PlayerController)
private:
	Player* m_player = nullptr;

public:
	//PlayerController(Actor* gameObject) : Component(gameObject) { }

	//void OnInit() override;
	void OnStart() override;
	void OnUpdate() override;

};

