#pragma once

#include "Game.h"
#include "MeshComponent.h"

class LineComponent : public MeshComponent {
public:

	void OnInit() override { 
		MeshComponent::OnInit();
	}

	void SetPoint(Vector3 worldPos, Vector4 color) {
		std::vector<Mesh4::Vertex> verteces;
		std::vector<int> indeces = { 0, 1 };

		auto& vertex1 = verteces.emplace_back();
		vertex1.position = Vector4(Vector3::Zero);
		vertex1.color = color;

		auto& vertex2 = verteces.emplace_back();
		vertex2.position = Vector4(worldPos);
		vertex2.color = color;

		this->AddShape(&verteces, &indeces);
		this->SetMaterial(0, "../../data/engine/shaders/vertex_color.hlsl");
		this->mesh()->topology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
	}

	void SetPoints(Vector3 begin, Vector3 end, Vector4 color) {
		std::vector<Mesh4::Vertex> verteces;
		std::vector<int> indeces = { 0, 1 };

		auto& vertex1 = verteces.emplace_back();
		vertex1.position = Vector4(begin);
		vertex1.color = color;

		auto& vertex2 = verteces.emplace_back();
		vertex2.position = Vector4(end);
		vertex2.color = color;

		this->AddShape(&verteces, &indeces);
		this->SetMaterial(0, "../../data/engine/shaders/vertex_color.hlsl");
		this->mesh()->topology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
	}
};
