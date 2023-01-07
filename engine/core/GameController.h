#pragma once

#include "Actor.h"
#include "InputDevice.h"

class CameraComponent;
class Player;
class ShadowPass;

class GameController : public Component {
	PURE_COMPONENT(GameController)
private:
	//CameraComponent* m_defaultCamera;
	//CameraComponent* m_playerCamera;

	Player* m_player;

	Actor* m_ñenter;
	Actor* m_collider;

	DelegateHandle m_mouseMoveHandle;

	//ShadowPass* m_shadowPass;

public:
	//GameController(Actor* actor) : Component(actor) {}

	void OnInit() override;
	void OnDestroy() override;

	void OnUpdate() override;

	Actor* CreatePlayer();

private:
	void m_OnMouseMove(const InputDevice::MouseMoveArgs& args);

};
