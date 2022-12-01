#pragma once

#include <vector>
#include <string>

#include "SimpleMath.h"

#include "Forms.h"
#include "Mesh.h"
#include "Game.h"
#include "ImageAsset.h"
#include "Material.h"
#include "Mesh.h"


class ImageComponent : public Component {
	PURE_COMPONENT(ImageComponent);
public:
	std::string shaderPath = "../../data/engine/shaders/image.hlsl";

private:
	std::vector<const Material*> m_materials;

	Mesh4 m_mesh;
	Material m_material;

	bool m_visible = true;

public:
	//ImageComponent(GameObject* gameObject) : Component(gameObject) {}

	void visibility(bool value) { m_visible = value; }
	bool visibility() { return m_visible; }

	void OnInit() override;
	void OnStart() override;
	void OnDrawUI() override;

	void SetImage(const ImageAsset::Image* image);
	void SetImage(comptr<ID3D11Texture2D> texture2D);

	void size(Vector2 size);
	Vector2 size();

private:
	void m_GenerateForm(std::vector<Mesh4::Vertex>* verteces, std::vector<int>* indeces);
	void m_InitMesh(int width, int height);

};
