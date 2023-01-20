#pragma once

#include <vector>
#include <string>
#include <list>

#include <d3d11.h>
#include <SimpleMath.h>

#include "Forms.h"
#include "Mesh.h"
#include "Actor.h"
#include "MaterialAlias.h"
#include "IShadowCaster.h"
#include "Refs.h"


using namespace DirectX::SimpleMath;

class Render;
class RenderPass;
class MeshAsset;
class Material;

FUNC(MeshComponent, SetPreInitMesh, void)(CppRef compRef, CppRef meshRef);
FUNC(MeshComponent, SetPreInitMaterials, void)(CppRef compRef, size_t* matRefs, int count);

class MeshComponent : public Component, public IShadowCaster {
	COMPONENT(MeshComponent);

	FRIEND_FUNC(MeshComponent, SetPreInitMesh, void)(CppRef compRef, CppRef meshRef);
	FRIEND_FUNC(MeshComponent, SetPreInitMaterials, void)(CppRef compRef, size_t* matRefs, int count);

public:
	bool isDebug = false;
	bool visible = true;

	Vector3 meshScale = Vector3::One;
	Vector3 meshOffset = Vector3::Zero;

private:
	Render* m_render;
	MeshAsset* m_meshAsset;
	Mesh4* m_dynamicMesh = nullptr;
	const Mesh4* m_mesh = nullptr;
	std::vector<const Material*> m_materials;
	std::vector<Material*> m_dynamicMaterials;
	std::vector<Pass::ShapeIter> m_shapeIters;

	bool m_castShadow = true;
	Pass::ShadowCaster m_shadowCaster;

	const Mesh4* m_preinitMesh = nullptr;
	std::vector<const Material*> m_preinitMaterials;

	int m_meshVersion = 0;

private:
	static bool mono_inited;
	static mono::mono_method_invoker<void(CsRef, CppRef)> mono_SetFromCpp;

public:
	bool IsDynamic() { return m_dynamicMesh != nullptr; }
	bool IsStatic() { return m_dynamicMesh == nullptr; }
	bool IsDrawable() { return visible && m_mesh != nullptr; }
	int MaterialCount() { return m_materials.size(); }

	bool castShadow() { return m_castShadow; }
	void castShadow(bool value);

	const Mesh4* mesh() { return m_mesh; }
	void mesh(const Mesh4* mesh) { m_SetMesh(mesh, false); };

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

	void OnPreInit();

	void OnInit() override;
	void OnDestroy() override;

	void OnInitDisabled() override  { OnInit(); }
	void OnDestroyDisabled() override  { OnDestroy(); }

	void OnDraw() override;
	void OnDrawShape(int index) override;
	//void OnDrawDebug() override;
	
	void OnDrawShadow(RenderPass* renderPass, const Vector3& scale) override;
	Component* GetComponent() override { return this; }

private:
	void m_SetMaterialsFromMesh();
	void m_OnMeshReload();
	void m_InitMono();
	void m_InitDynamic();
	void m_Draw(RenderPass* renderPass = nullptr, const Vector3& scale = Vector3::One);
	void m_DeleteResources();
	void m_DeleteMaterials();
	void m_DeleteLocalDynamicMaterial(int index);
	void m_FillByDefaultMaterial(int targetSize);

	void m_RegisterShapesWithMaterial(int materialIndex);
	void m_UnRegisterShapesWithMaterial(int materialIndex);

	void m_SetMesh(const Mesh4* mesh, bool isDynamic);

};
DEC_COMPONENT(MeshComponent);

PROP_GET(MeshComponent, bool, IsDynamic);
PROP_GET(MeshComponent, bool, IsStatic);
PROP_GET(MeshComponent, int, MaterialCount);
PROP_GETSET(MeshComponent, bool, castShadow);

FUNC(MeshComponent, SetFromCs, void)(CppRef compRef, CppRef meshRef);

FUNC(MeshComponent, AddShape, void)(CppRef compRef, Mesh4::Vertex* verteces, int vength, int* indeces, int ilength, int matIndex);

FUNC(MeshComponent, RemoveMaterials, void)(CppRef compRef);
FUNC(MeshComponent, RemoveMaterial, void)(CppRef compRef, int index);
FUNC(MeshComponent, ClearMesh, void)(CppRef compRef);

FUNC(MeshComponent, SetMaterial, void)(CppRef compRef, size_t index, CppRef materialRef);
FUNC(MeshComponent, GetMaterial, CppRef)(CppRef compRef, size_t index);

FUNC(MeshComponent, OnPreInit, void)(CppRef compRef);




