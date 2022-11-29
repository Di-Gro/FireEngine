#include "LinedPlain.h"

#include <d3d11.h>
#include <SimpleMath.h>

#include "MeshComponent.h"
#include "Game.h"

using namespace DirectX::SimpleMath;


void LinedPlain::OnInit() {
	std::vector<Vector4> points;
	std::vector<int> indeces;
	m_GeneratePoints(points, indeces);

	std::vector<Mesh4::Vertex> verteces;
	verteces.reserve(points.size());

	for (int i = 0; i < points.size(); ++i) {
		Mesh4::Vertex vertex;
		vertex.position = Vector4(Vector3(points[i]));
		vertex.color = m_color;
		verteces.push_back(vertex);
	}

	m_meshComponent = AddComponent<MeshComponent>();
	m_meshComponent->AddShape(&verteces, &indeces);
	m_meshComponent->mesh()->topology = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
}

void LinedPlain::OnDestroy() {
	m_meshComponent->Destroy();
}

void LinedPlain::m_GeneratePoints(std::vector<Vector4>& points, std::vector<int>& indexes) {

	float halfSize = m_size / 2;
	for (float delta = 0.0f; delta <= halfSize; delta += m_tileSize) {

		auto list = {
			Vector4(delta, 0.0f, -halfSize, 1.0f),// m_color,
			Vector4(delta, 0.0f, halfSize, 1.0f), //m_color,
			Vector4(-delta, 0.0f, -halfSize, 1.0f), //m_color,
			Vector4(-delta, 0.0f, halfSize, 1.0f), //m_color,
			Vector4(-halfSize, 0.0f, delta, 1.0f), //m_color,
			Vector4(halfSize, 0.0f, delta, 1.0f), //m_color,
			Vector4(-halfSize, 0.0f, -delta, 1.0f), //m_color,
			Vector4(halfSize, 0.0f, -delta, 1.0f), //m_color
		};

		int s = points.size();
		points.insert(points.end(), list);
		indexes.insert(indexes.end(), { s, s + 1, s + 2, s + 3, s + 4, s + 5, s + 6, s + 7 });
	}
}
