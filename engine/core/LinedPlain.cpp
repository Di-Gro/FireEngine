#include "LinedPlain.h"

#include <d3d11.h>
#include <SimpleMath.h>

#include "MeshComponent.h"
#include "Game.h"
#include "Actor.h"
#include "Material.h"
#include "MeshAsset.h"

DEF_PURE_COMPONENT(LinedPlain, RunMode::EditOnly);

void LinedPlain::OnInit() {
	m_material = game()->meshAsset()->CreateDynamicMaterial("Lined Plain material", Assets::ShaderDiffuseColor);
}

void LinedPlain::OnStart() {
	std::vector<Mesh4::Vertex> verteces;
	std::vector<int> indeces;
	m_GeneratePoints(verteces, indeces);

	m_meshComponent = AddComponent<MeshComponent>();
	m_meshComponent->AddShape(&verteces, &indeces);
	m_meshComponent->mesh()->topology = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
	m_meshComponent->SetMaterial(0, m_material);
	m_meshComponent->castShadow(false);

	m_material->data.diffuseColor = color;
	m_material->data.specular = 0;
}

void LinedPlain::OnDestroy() {
	m_meshComponent->ClearMesh();
	m_meshComponent->Destroy();

	game()->meshAsset()->DeleteDynamicMaterial(m_material);
	m_material = nullptr;
}

void LinedPlain::m_GeneratePoints(std::vector<Mesh4::Vertex>& points, std::vector<int>& indexes) {
	using V4 = Mesh4::Vertex;

	float halfSize = size / 2;
	for (float delta = 0.0f; delta <= halfSize; delta += tileSize) {

		auto list = {
			V4({delta, 0.0f, -halfSize}),
			V4({delta, 0.0f, halfSize}),
			V4({-delta, 0.0f, -halfSize}),
			V4({-delta, 0.0f, halfSize}),
			V4({-halfSize, 0.0f, delta}),
			V4({halfSize, 0.0f, delta}),
			V4({-halfSize, 0.0f, -delta}),
			V4({halfSize, 0.0f, -delta}),
		};

		int s = points.size();
		points.insert(points.end(), list);
		indexes.insert(indexes.end(), { s, s + 1, s + 2, s + 3, s + 4, s + 5, s + 6, s + 7 });
	}
}
