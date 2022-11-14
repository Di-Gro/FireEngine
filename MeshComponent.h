#pragma once

#include <vector>
#include <string>

#include <d3d11.h>
#include <SimpleMath.h>

#include "Forms.h"
#include "Mesh.h"
#include "Game.h"

using namespace DirectX::SimpleMath;


class MeshComponent : public Component {
public:

	bool isDebug = false;

private:
	const Mesh4* m_mesh = nullptr;
	const std::vector<const Material*>* m_materials;
	std::vector<Material> m_selfMaterials;
	std::vector<const Material*> m_selfConstMaterials;

	bool m_visible = true;
	bool m_isMeshHolder = false;

public:
	MeshComponent(GameObject* gameObject) : Component(gameObject) {}

	const Mesh4* mesh() { return m_mesh; }

	void OnDraw() override;
	void OnDrawDebug() override;
	void OnDestroy() override;

	void CreateMesh(std::string meshPath);
	void CreateMesh(
		std::vector<Mesh4::Vertex>* verteces,
		std::vector<int>* indeces,
		fs::path shaderPath = "data/engine/shaders/default.hlsl");

	void ShowNormals(std::vector<Mesh4::Vertex>* verteces);

	void SetVisibility(bool value) { m_visible = value; }

private:
	void m_Draw();

};
