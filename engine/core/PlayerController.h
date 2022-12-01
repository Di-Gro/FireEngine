#pragma once

#include "Game.h"

class Player;
class PlayerCamera;

class PlayerController : public Component {
	PURE_COMPONENT(PlayerController)
private:
	Player* m_player = nullptr;

public:
	//PlayerController(GameObject* gameObject) : Component(gameObject) { }

	//void OnInit() override;
	void OnStart() override;
	void OnUpdate() override;

};

