#pragma once

#include "Game.h"

class CameraComponent;
class Player;


class GameController : public Component {
	PURE_COMPONENT; 
private:
	CameraComponent* m_defaultCamera;
	CameraComponent* m_playerCamera;

	Player* m_player;

	GameObject* m_ñenter;
	GameObject* m_collider;

	DelegateHandle m_mouseMoveHandle;

public:
	//GameController(GameObject* gameObject) : Component(gameObject) {}

	void OnInit() override;
	void OnDestroy() override;

	void OnUpdate() override;

	GameObject* CreatePlayer();

private:
	void m_OnMouseMove(const InputDevice::MouseMoveArgs& args);

};
