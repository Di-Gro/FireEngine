#pragma once

#include <d3d11.h>
#include <SimpleMath.h>

#include "MeshComponent.h"
#include "Forms.h"

using namespace DirectX::SimpleMath;

class MeshComponent;

class LinedPlain : public Component {
private:

	float m_size = 10'000.0f;
	float m_tileSize = 30.0f;
	Vector4 m_color = MyColors::SkyWhite; //{231 / 255.0f, 159 / 255.0f, 196 / 255.0f, 1.0f}; //{ 0.8f, 0.0f, 0.0f, 0.0f }; 

	MeshComponent* m_meshComponent;

public:
	//LinedPlain(GameObject* gameObject) : Component(gameObject) {}

	void OnInit() override;
	void OnDestroy() override;

private:
	void m_GeneratePoints(std::vector<Vector4>& points, std::vector<int>& indexes);

};

