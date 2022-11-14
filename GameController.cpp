#include "GameController.h"

#include <d3d11.h>
#include <SimpleMath.h>
#include <string>
#include "FileSystem.h"

#include <fstream>
#include <sstream>

#include "Game.h"
#include "LinedPlain.h"
#include "Forms.h"
#include "RotatorComponent.h"
#include "MoveController.h"
#include "TransformTest.h"
#include "ScalerComponent.h"
#include "MeshComponent.h"
#include "Player.h"
#include "PlayerController.h"
#include "PlayerCamera.h"
#include "Random.h"
#include "Collider.h"
#include "AxisDrawer.h"
#include "LineComponent.h"
#include "DirectionLight.h"
#include "TestComponent.h"

using namespace DirectX::SimpleMath;


static inline Vector3 deg(Vector3 vec) {
	return Vector3(deg(vec.x), deg(vec.y), deg(vec.z));
}

static std::string ToString(Transform& transform) {
	std::string str;

	str += "local position: " + ToString(transform.localPosition()) + "\n";
	str += "local rotation: " + ToString(deg(transform.localRotation())) + "\n";

	str += "world position: " + ToString(transform.worldPosition()) + "\n";
	str += "world rotation: " + ToString(deg(transform.worldRotation())) + "\n";

	return str;
}

GameObject* GameController::CreatePlayer() {

	auto player = CreateGameObject("player");
	player->AddComponent<Player>();
	player->AddComponent<PlayerController>();
	
	auto cameraRoot = CreateGameObject("camera root");
	cameraRoot->SetParent(player);
	cameraRoot->transform.localPosition({ 0, 0, 0 });

	auto camera = CreateGameObject("player camera");
	camera->SetParent(cameraRoot);
	camera->transform.localPosition({ 0, 0, 300 });
	camera->AddComponent<PlayerCamera>();

	auto test = player->AddComponent<TestComponent>();
	
	return player;
}

static Vector3 GetRandomPos(Vector3 point, float minRadius, float maxRadius) {
	auto random = Random(); 
	float radius = std::lerp(minRadius, maxRadius, random.Value());
	float angle = random.Value() * 360.0f;

	
	auto rot = Quaternion::CreateFromYawPitchRoll(angle, 0, 0);
	Quaternion inv;
	rot.Inverse(inv);

	Quaternion p0{ 0, 0, -1, 0 };

	auto p1 = rot * p0 * inv;

	Vector3 dir{ p1.x, p1.y, p1.z };
	return dir * radius;
}

void GameController::OnInit() {

	AddComponent<LineComponent>()->SetPoint(Vector3::Forward * 100, { 1, 1, 1, 1 });
	AddComponent<LineComponent>()->SetPoint(Vector3::Up * 100, { 0, 1, 0, 1 });
	AddComponent<LineComponent>()->SetPoint(Vector3::Right * 100, { 1, 0, 0, 1 });

	m_defaultCamera = game()->mainCamera();
	m_defaultCamera->transform.localPosition({ 0, 100,-300 });

	m_player = CreatePlayer()->GetComponent<Player>();
	m_playerCamera = m_player->GetComponentInChild<PlayerCamera>();

	std::vector<GameObject*> newObjects;
	game()->meshAsset()->LoadScene("data/assets/levels/farm", &newObjects);
	
	for (auto gobj : newObjects) {
		auto attachable = gobj->GetComponent<Attachable>();
		if (attachable != nullptr)
			attachable->player = m_player;
	}

	
	auto sprite = CreateGameObject("UI Image")->AddComponent<ImageComponent>();
	sprite->SetImage(game()->lighting()->directionLight()->shadowRT.depthTexture());

	auto imgSize = sprite->size() * 0.1f;

	sprite->transform.localPosition({ 20, 1080 - imgSize.y - 35, 0 });
	sprite->size(imgSize);

	auto test = m_player->GetComponentInChild<TestComponent>();
	test->targetImage = sprite;

	game()->hotkeys()->RegisterHotkey(Keys::Esc);
	game()->hotkeys()->RegisterHotkey(Keys::Tab);
	game()->hotkeys()->RegisterHotkey(Keys::F);
	game()->hotkeys()->RegisterHotkey(Keys::R);

	m_mouseMoveHandle = game()->input()->MouseMove.AddRaw(this, &GameController::m_OnMouseMove);
}

void GameController::OnDestroy() {
	game()->input()->MouseMove.Remove(m_mouseMoveHandle);
}

void GameController::OnUpdate() {
	auto hotkeys = game()->hotkeys();
	if (hotkeys->Is(Keys::Esc, KeyState::Press)) {
		std::cout << "Exit" << std::endl;
		game()->Exit(0);
	}

	if (hotkeys->Is(Keys::Tab, KeyState::Press)) {
		std::cout << "send game message: ";
		std::string msg;
		std::getline(std::cin, msg);
		game()->SendGameMessage(msg);
	}

	if (hotkeys->Is(Keys::F, KeyState::Press)) {
		if (m_defaultCamera->IsAttached()) {
			m_playerCamera->Attach();
		}
		else if (m_playerCamera->IsAttached()) {
			m_defaultCamera->Attach();
		}
	}

	if (hotkeys->Is(Keys::R, KeyState::Press)) {
		game()->SendGameMessage("recompile");
	}
}


void GameController::m_OnMouseMove(const InputDevice::MouseMoveArgs& args) {

	static int v1 = 0;
	static float v2 = 0, v3 = 0;
	static D3D11_CULL_MODE CullMode = D3D11_CULL_FRONT; //D3D11_CULL_BACK;

	if (args.WheelDelta == 0)
		return;

	int sense1 = 50;
	float sense2 = 0.1f;
	float sense3 = 0.1f;

	bool hasKey1 = game()->input()->IsKeyDown(Keys::Key1);
	bool hasKey2 = game()->input()->IsKeyDown(Keys::Key2);
	bool hasKey3 = game()->input()->IsKeyDown(Keys::Key3);
	bool hasKey4 = game()->input()->IsKeyDown(Keys::Key4);

	int sign = args.WheelDelta > 0 ? 1 : -1;
	if (hasKey1)
		v1 += sign * sense1;

	if (hasKey2)
		v2 += sign * sense2;

	if (hasKey3)
		v3 += sign * sense3;

	if (hasKey4)
		CullMode = CullMode == D3D11_CULL_FRONT ? D3D11_CULL_BACK : D3D11_CULL_FRONT;

	if (hasKey1 || hasKey2 || hasKey3 || hasKey4) {
		game()->lighting()->directionLight()->shadowRT.ResetRastState(v1, v2, v3, CullMode);
		std::cout << v1 << ", " << v2 << ", " << v3 << std::endl;
	}
}