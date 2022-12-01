#pragma once

#include "Game.h"
#include "ImageComponent.h"
#include "DirectionLight.h"

class TestComponent : public Component {
	PURE_COMPONENT(TestComponent)
public:
	ImageComponent* targetImage = nullptr;

private:
	ImageComponent* m_cursor = nullptr;
	
public:
	//TestComponent(GameObject* gameObject) : Component(gameObject) { }

	void OnInit() override {
		m_cursor = CreateGameObject("Test Cursor")->AddComponent<ImageComponent>();
		m_cursor->SetImage(game()->imageAsset()->Get(ImageAsset::RUNTIME_IMG_2X2_RGBA_1001));
		m_cursor->size({ 3, 3 });
	}

	void OnUpdate() override {
		auto directionLight = game()->lighting()->directionLight();
		auto camera = directionLight->camera();

		auto wpos = transform->worldPosition();
		//auto transMatrix = Matrix::CreateTranslation(wpos) * camera->cameraMatrix();
		//auto devicePos = transMatrix.Translation();
		//auto uvw = (devicePos + Vector3::One) / 2;

		//auto transMatrix = camera->cameraMatrix() * Matrix::CreateTranslation(Vector3::One) * Matrix::CreateScale({ 0.5, 0.5, 0.5 });
		auto transMatrix = directionLight->uvMatrix();// *Matrix::CreateTranslation(Vector3::One)* Matrix::CreateScale({ 0.5, 0.5, 0.5 });;
		//auto m = transMatrix * Matrix::CreateTranslation(Vector3::One) * Matrix::CreateScale({ 0.5, 0.5, 0.5 });

		auto m = Matrix::CreateTranslation(wpos) * transMatrix;

		auto uvw = m.Translation();

		if (targetImage != nullptr) {
			auto tPos = targetImage->transform->worldPosition();
			auto tSize = targetImage->size();
			auto ltPos = Vector3(tSize) * uvw;

			m_cursor->transform->localPosition(tPos + ltPos);
		}
	}

};

DEF_PURE_COMPONENT(TestComponent);