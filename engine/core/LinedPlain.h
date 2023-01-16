#pragma once

#include "SimpleMath.h"

#include "CSBridge.h"
#include "Actor.h"
#include "Mesh.h"

class MeshComponent;
class Material;

class LinedPlain : public Component {
	PURE_COMPONENT(LinedPlain);

public:
	Vector3 color = { 0.13f, 0.14f, 0.15f };
	float size = 10'000.0f;
	float tileSize = 30.0f;

private:
	MeshComponent* m_meshComponent;
	Material* m_material = nullptr;

public:
	void OnInit() override;
	void OnStart() override;
	void OnDestroy() override;

private:
	void m_GeneratePoints(std::vector<Mesh4::Vertex>& points, std::vector<int>& indexes);

};

