#pragma once

#include "Game.h"

class CameraComponent;
class Player;


class GameController : public Component {
	PURE_COMPONENT(GameController)
private:
	CameraComponent* m_defaultCamera;
	CameraComponent* m_playerCamera;

	Player* m_player;

	Actor* m_ñenter;
	Actor* m_collider;

	DelegateHandle m_mouseMoveHandle;

public:
	//GameController(Actor* actor) : Component(actor) {}

	void OnInit() override;
	void OnDestroy() override;

	void OnUpdate() override;

	Actor* CreatePlayer();

private:
	void m_OnMouseMove(const InputDevice::MouseMoveArgs& args);

};
