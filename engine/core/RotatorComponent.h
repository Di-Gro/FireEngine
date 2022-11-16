#pragma once
#include <string>

#include <d3d11.h>
#include <SimpleMath.h>

#include "Game.h"

#include "Random.h"
#include "Math.h"

using namespace DirectX::SimpleMath;


class RotatorComponent : public Component {
private:
	Vector3 angles;
	Vector3 axis = Vector3::Up;
	Vector3 perp = Vector3::Right;
	float angle = 0;
	float tatalAngle = 0;

	bool byAxis = true;
	bool printTransform = false;

public:

	void RotateByAxis(Vector3 axis, float angle) {
		this->axis = axis;
		this->angle = angle;
		byAxis = true;
	}

	void RotateByEuler(Vector3 angles) {
		this->angles = angles;
		byAxis = false;
	}

	Vector3 GetPerp() {
		return perp;
	}

	RotatorComponent(GameObject* gameObject) : Component(gameObject) {}

	void OnInit() override {
		auto random = Random();
		axis.x = random.Value() * 2 - 1;
		axis.y = random.Value() * 2 - 1;
		axis.z = random.Value() * 2 - 1;

		angle = rad(random.Range(40, 90));

		auto dir = axis;
		dir.Normalize();
		perp = dir.Cross(Vector3::Forward).Normalized();
	}

	void OnUpdate() override {
		if (byAxis) {
			tatalAngle += angle * game()->deltaTime();
			transform.localRotation(axis, tatalAngle);
		}
		else {
			transform.localRotation(transform.localRotation() + angles * game()->deltaTime());
		}

		if(printTransform)
			game()->SendGameMessage(std::to_string(gameObject()->Id()) + " tr");
	}

	void RecieveGameMessage(const std::string& msg) override {
		if (msg == "rotcom.print.tr")
			printTransform = !printTransform;
	}


};

