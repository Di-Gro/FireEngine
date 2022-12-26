#include "PlayerController.h"

#include <iostream>

#include "Game.h"
#include "HotKeys.h"

#include "Player.h"
#include "PlayerCamera.h"

DEF_PURE_COMPONENT(PlayerController);

void PlayerController::OnStart() {
	m_player = GetComponent<Player>();
	if (!m_player)
		std::cout << "PlayerController: Player component don't found.\n";
}

void PlayerController::OnUpdate() {
	if (!m_player || !game()->inFocus)
		return;

	auto input = game()->input();
	if (input->IsKeyDown(Keys::RightButton))
		return;

	auto axis = Vector3::Zero;
	if (input->IsKeyDown(Keys::W)) axis += Vector3::Right;
	if (input->IsKeyDown(Keys::S)) axis += Vector3::Left;
	if (input->IsKeyDown(Keys::A)) axis += Vector3::Forward;
	if (input->IsKeyDown(Keys::D)) axis += Vector3::Backward;
	axis.Normalize();
	if (input->IsKeyDown(Keys::LeftShift)) axis *= 2;

	if (axis != Vector3::Zero)
		m_player->Move(axis);
}