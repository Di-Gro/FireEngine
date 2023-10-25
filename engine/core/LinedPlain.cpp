#include "LinedPlain.h"

#include <d3d11.h>
#include <SimpleMath.h>

#include "MeshComponent.h"
#include "Game.h"
#include "Actor.h"
#include "Assets.h"
#include "MaterialAsset.h"
#include "MeshAsset.h"
#include "Vertex.h"

DEF_PURE_COMPONENT(LinedPlain, RunMode::EditOnly);

void LinedPlain::OnInit() {
	m_material = MaterialAsset::CreateDynamic(game(), "Lined Plain material", Assets::ShaderDiffuseColor);
}

void LinedPlain::OnStart() {
	std::vector<Vertex> verteces;
	std::vector<int> indeces;
	m_GeneratePoints(verteces, indeces);

	m_meshComponent = AddComponent<MeshComponent>();
	m_meshComponent->AddShape(&verteces, &indeces);
	m_meshComponent->mesh()->resource.topology = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
	m_meshComponent->SetMaterial(0, m_material);
	m_meshComponent->castShadow(false);

	m_material->resource.data.diffuseColor = color;
	m_material->resource.data.specular = 0;
}

void LinedPlain::OnDestroy() {
	if (m_meshComponent != nullptr) {
		m_meshComponent->ClearMesh();
		m_meshComponent->Destroy();
	}

	MaterialAsset::DeleteDinamic(game(), m_material);
	m_material = nullptr;
}

void LinedPlain::m_GeneratePoints(std::vector<Vertex>& points, std::vector<int>& indexes) {
	float halfSize = size / 2;
	for (float delta = 0.0f; delta <= halfSize; delta += tileSize) {

		auto list = {
			Vertex({delta, 0.0f, -halfSize}),
			Vertex({delta, 0.0f, halfSize}),
			Vertex({-delta, 0.0f, -halfSize}),
			Vertex({-delta, 0.0f, halfSize}),
			Vertex({-halfSize, 0.0f, delta}),
			Vertex({halfSize, 0.0f, delta}),
			Vertex({-halfSize, 0.0f, -delta}),
			Vertex({halfSize, 0.0f, -delta}),
		};

		int s = points.size();
		points.insert(points.end(), list);
		indexes.insert(indexes.end(), { s, s + 1, s + 2, s + 3, s + 4, s + 5, s + 6, s + 7 });
	}
}
