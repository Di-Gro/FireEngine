#pragma once

#include <vector>
#include <string>
#include <list>

#include <d3d11.h>
#include <SimpleMath.h>

#include "Forms.h"
#include "Mesh.h"
#include "Game.h"

using namespace DirectX::SimpleMath;


class MeshComponent : public Component {
	COMPONENT(MeshComponent)

public:
	bool isDebug = false;
	bool visible = true;

	Vector3 meshScale = Vector3::One;

private:
	Mesh4* m_dynamicMesh = nullptr;
	const Mesh4* m_mesh = nullptr;
	std::vector<const Material*> m_materials;
	std::vector<Material*> m_dynamicMaterials;

	std::list<Component*>::iterator m_renderHandle;

private:
	static bool mono_inited;
	static mono::mono_method_invoker<void(CsRef, CppRef)> mono_SetFromCpp;

public:
	const Mesh4* mesh() {
		return m_mesh;
	}

	bool IsDynamic() { return m_dynamicMesh != nullptr; }
	bool IsStatic() { return m_dynamicMesh == nullptr; }
	int MaterialCount() { return m_materials.size(); }

	void mesh(const Mesh4* mesh);

	void SetMeshFromCs(const Mesh4* mesh);

	void AddShape(
		std::vector<Mesh4::Vertex>* verteces, 
		std::vector<int>* indeces, 
		size_t materialIndex = 0);
	
	void AddShape(
		Mesh4::Vertex* verteces,
		int vertecesLength,
		int* indeces,
		int indecesLength,
		int materialIndex);
	
	void SetMaterial(size_t index, const fs::path& shaderPath);
	void SetMaterial(size_t index, const Material* other);

	const Material* GetMaterial(size_t index);

	void RemoveMaterial(size_t index);
	void RemoveMaterials();
	void ClearMesh();

	void OnInit() override;
	void OnDraw() override;
	void OnDrawDebug() override;
	void OnDestroy() override;

private:
	void m_SetMaterialsFromMesh();
	void m_InitMono();
	void m_InitDynamic();
	void m_Draw();
	void m_DeleteResources();
	void m_DeleteMaterials();
	void m_DeleteLocalDynamicMaterial(int index);
	void m_FillByDefaultMaterial(int targetSize);

};
DEC_COMPONENT(MeshComponent);

PROP_GET(MeshComponent, bool, IsDynamic)
PROP_GET(MeshComponent, bool, IsStatic)
PROP_GET(MeshComponent, int, MaterialCount)

FUNC(MeshComponent, SetFromCs, void)(CppRef compRef, CppRef meshRef);

FUNC(MeshComponent, AddShape, void)(CppRef compRef, Mesh4::Vertex* verteces, int vength, int* indeces, int ilength, int matIndex);

FUNC(MeshComponent, RemoveMaterials, void)(CppRef compRef);
FUNC(MeshComponent, RemoveMaterial, void)(CppRef compRef, int index);
FUNC(MeshComponent, ClearMesh, void)(CppRef compRef);

FUNC(MeshComponent, SetMaterial, void)(CppRef compRef, size_t index, CppRef materialRef);
FUNC(MeshComponent, GetMaterial, CppRef)(CppRef compRef, size_t index);



