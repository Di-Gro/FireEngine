#include "MeshComponent.h"
#include <list>

#include <SimpleMath.h>

#include "Game.h"
#include "Render.h"
#include "MeshAsset.h"
#include "Actor.h"
#include "Random.h"

#include "CameraComponent.h"
#include "LineComponent.h"

using namespace DirectX::SimpleMath;

bool MeshComponent::TempVisible = false;
bool MeshComponent::mono_inited = false;
mono::mono_method_invoker<void(CsRef, CppRef)> MeshComponent::mono_SetFromCpp;


void MeshComponent::OnInit() {
	m_render = game()->render();
	m_meshAsset = game()->meshAsset();
	m_InitMono();

	m_shadowCaster = m_render->AddShadowCaster(this);

	if (m_preinitMesh != nullptr) {
		mesh(m_preinitMesh);
		m_preinitMesh = nullptr;
	}
	if (m_preinitMaterials.size() > 0) {
		for (int i = 0; i < m_preinitMaterials.size() || i < m_materials.size(); i++)
			SetMaterial(i, m_preinitMaterials[i]);
		m_preinitMaterials.clear();
	}
}

void MeshComponent::m_InitMono() {
	if (mono_inited)
		return;

	auto type = game()->mono()->GetType("Engine", "MeshComponent");
	mono_SetFromCpp = mono::make_method_invoker<void(CsRef, CppRef)>(type, "cpp_SetFromCpp");

	mono_inited = true;
}

void MeshComponent::m_InitDynamic() {
	if (IsDynamic())
		return;

	auto assetId = "DynamicMesh_" + std::to_string(Random().Int());
	auto assetIdHash = game()->assets()->GetCsHash(assetId);
	auto meshCppRef = Mesh4_PushAsset(CppRefs::GetRef(game()), assetId.c_str(), assetIdHash);
	auto newMesh = CppRefs::ThrowPointer<Mesh4>(meshCppRef);

	*newMesh = *m_mesh;
			
	m_SetMesh(newMesh, true);
}

void MeshComponent::OnDestroy() {
	m_DeleteResources();

	if(m_castShadow)
		m_render->RemoveShadowCaster(m_shadowCaster);
}

void MeshComponent::RemoveMaterial(size_t index) {
	if (index > m_mesh->maxMaterialIndex())
		return;

	// ������� ������ ��������
	m_UnRegisterShapesWithMaterial(index);
	m_DeleteLocalDynamicMaterial(index);

	// ������ ����������� ��������
	auto defaultMaterial = m_meshAsset->GetStaticMaterial(MeshAsset::materialDefault);
	m_materials[index] = defaultMaterial;
	m_RegisterShapesWithMaterial(index);
}

void MeshComponent::RemoveMaterials() {
	m_DeleteMaterials();

	if (m_mesh != nullptr)
		m_FillByDefaultMaterial(m_mesh->maxMaterialIndex() + 1);
}

void MeshComponent::ClearMesh() {
	m_DeleteResources();

	if (m_mesh != nullptr)
		m_FillByDefaultMaterial(m_mesh->maxMaterialIndex() + 1);
}

void MeshComponent::m_DeleteResources() {
	m_DeleteMaterials();
	if (IsDynamic()) {
		game()->assets()->Pop(m_dynamicMesh->assetIdHash());
		delete m_dynamicMesh;
		m_dynamicMesh = nullptr;
		m_mesh = nullptr;
	}
}

void MeshComponent::m_DeleteMaterials() {
	for (int i = 0; i < m_materials.size(); ++i) {
		m_UnRegisterShapesWithMaterial(i);
		m_DeleteLocalDynamicMaterial(i);
	}

	m_materials.clear();
	m_dynamicMaterials.clear();
	m_shapeIters.clear();
}

void MeshComponent::m_DeleteLocalDynamicMaterial(int index) {
	auto localMaterial = m_dynamicMaterials[index];
	if (localMaterial == nullptr)
		return;

	m_meshAsset->DeleteDynamicMaterial(localMaterial);

	m_materials[index] = nullptr;
	m_dynamicMaterials[index] = nullptr;
}

void MeshComponent::AddShape(
	std::vector<Mesh4::Vertex>* verteces, 
	std::vector<int>* indeces, 
	size_t materialIndex) 
{
	m_InitDynamic();

	m_dynamicMesh->AddShape(verteces, indeces, m_render, materialIndex);

	m_FillByDefaultMaterial(m_dynamicMesh->maxMaterialIndex() + 1);
}

void MeshComponent::AddShape(
	Mesh4::Vertex* verteces,
	int vertecesLength,
	int* indeces,
	int indecesLength,
	int materialIndex) 
{
	m_InitDynamic();

	m_dynamicMesh->AddShape(verteces, vertecesLength, indeces, indecesLength, m_render, materialIndex);

	m_FillByDefaultMaterial(m_dynamicMesh->maxMaterialIndex() + 1);
}

void MeshComponent::SetMaterial(size_t index, const fs::path& shaderPath) {
	m_InitDynamic();

	if (index > m_mesh->maxMaterialIndex())
		return;

	assert(index < m_dynamicMaterials.size());

	// ������� ��������� ��������
	m_UnRegisterShapesWithMaterial(index);
	m_DeleteLocalDynamicMaterial(index);

	// ������� ����� ��������
	auto material = m_meshAsset->CreateDynamicMaterial("Generated by MeshComponent", shaderPath);

	// ���������� ����� ��������
	m_materials[index] = material;
	m_dynamicMaterials[index] = material;
	m_RegisterShapesWithMaterial(index);
}

void MeshComponent::SetMaterial(size_t index, const Material* material) {
	if (index > m_mesh->maxMaterialIndex())
		return;

	assert(index < m_dynamicMaterials.size());

	// ������� ��������� ��������
	m_UnRegisterShapesWithMaterial(index);
	m_DeleteLocalDynamicMaterial(index);

	// ���������� ����� ��������
	m_materials[index] = material;
	m_dynamicMaterials[index] = nullptr;
	m_RegisterShapesWithMaterial(index);
}

const Material* MeshComponent::GetMaterial(size_t index) {
	if (index >= m_materials.size())
		return nullptr;

	return m_materials[index];
}

void MeshComponent::m_FillByDefaultMaterial(int targetSize) {
	if (m_materials.size() < targetSize) {
		auto defaultMaterial = m_meshAsset->GetStaticMaterial(MeshAsset::materialDefault);

		m_shapeIters.resize(m_mesh->shapeCount() > 0 ? m_mesh->shapeCount() : 1);

		for (int matIndex = m_materials.size(); matIndex < targetSize; matIndex++) {

			m_materials.push_back(defaultMaterial);
			m_dynamicMaterials.push_back(nullptr);

			m_RegisterShapesWithMaterial(matIndex);
		}
	}
}

void MeshComponent::m_SetMesh(const Mesh4* mesh, bool isDynamic) {
	m_DeleteResources();
	m_mesh = mesh;
	m_meshVersion = m_mesh->version;
	m_SetMaterialsFromMesh();

	if(isDynamic)
		m_dynamicMesh = (Mesh4*)mesh;

	if (csRef() > 0) {
		auto newRef = m_mesh != nullptr ? CppRefs::GetRef((void*)m_mesh) : RefCpp(0);
		mono_SetFromCpp(csRef(), newRef);
	}
}

void MeshComponent::SetMeshFromCs(const Mesh4* mesh) {
	m_DeleteResources();
	m_mesh = mesh;
	m_SetMaterialsFromMesh();
}

void MeshComponent::m_SetMaterialsFromMesh() {
	if (m_mesh == nullptr)
		return;

	// ����� ������ ��������� ����
	auto* staticMaterials = m_mesh->GetMaterials();
	auto newSize = staticMaterials->size();

	// ������� ������� ��� ����������
	m_materials.reserve(newSize);
	m_dynamicMaterials.reserve(newSize);

	// ������� ������ ���������� shape-��
	m_shapeIters.resize(m_mesh->shapeCount());
		
	for (int materialIndex = 0; materialIndex < newSize; materialIndex++) {
		auto material = staticMaterials->at(materialIndex);

		// ���������� ������ ��������� 
		m_materials.push_back(material);
		m_dynamicMaterials.push_back(nullptr);
		m_RegisterShapesWithMaterial(materialIndex);
	}

	// ���� ����� ���������� ������, ��� ������������,
	// ��������� ������������ �����������.
	m_FillByDefaultMaterial(m_mesh->maxMaterialIndex() + 1);
}

void MeshComponent::m_OnMeshReload() {
	std::cout << "MeshComponent::m_OnMeshReload() NotImplemented" << std::endl;
}

void MeshComponent::m_RegisterShapesWithMaterial(int materialIndex) {
	auto* material = m_materials[materialIndex];

	bool isNewDynamic = 
		m_mesh->shapeCount() == 0 &&
		m_materials.size() == 1 && 
		materialIndex == 0;

	if (isNewDynamic) {
		m_shapeIters[0] = m_render->RegisterShape(material, this, 0);
		return;
	}

	for (int shapeIndex = 0; shapeIndex < m_mesh->shapeCount(); shapeIndex++) {
		const auto* shape = &m_mesh->m_shapes[shapeIndex];

		if (shape->materialIndex == materialIndex)
			m_shapeIters[shapeIndex] = m_render->RegisterShape(material, this, shapeIndex);
	}
}

void MeshComponent::m_UnRegisterShapesWithMaterial(int materialIndex) {
	auto* material = m_materials[materialIndex];

	for (int shapeIndex = 0; shapeIndex < m_mesh->m_shapes.size(); shapeIndex++) {
		const auto* shape = &m_mesh->m_shapes[shapeIndex];

		if (shape->materialIndex == materialIndex)
			m_render->UnRegisterShape(material, m_shapeIters[shapeIndex]);
	}
}

void MeshComponent::castShadow(bool value) {
	if (m_castShadow && !value)
		m_render->RemoveShadowCaster(m_shadowCaster);

	if (!m_castShadow && value)
		m_shadowCaster = m_render->AddShadowCaster(this);

	m_castShadow = value;
}

void MeshComponent::OnDrawShadow(RenderPass* renderPass, const Vector3& scale) {
	if (!isDebug)
		m_Draw(renderPass, scale);
}

void MeshComponent::OnDraw() {
	if (!isDebug)
		m_Draw();
}

void MeshComponent::OnDrawDebug() {
	if (isDebug)
		m_Draw();
}

void MeshComponent::m_Draw(RenderPass* renderPass, const Vector3& scale) {
	if (!visible || m_mesh == nullptr)
		return;
	if (!TempVisible && !ingnore_TempVisible)
		return;
	if (m_meshVersion != m_mesh->version)
		m_OnMeshReload();

	auto camera = m_render->camera();
	auto cameraPosition = camera->worldPosition();
	auto worldMatrix = Matrix::CreateScale(meshScale * scale) * GetWorldMatrix();
	auto transMatrix = worldMatrix * camera->cameraMatrix();

	Mesh4::DynamicShapeData data;
	data.render = m_render;
	data.worldMatrix = &worldMatrix;
	data.transfMatrix = &transMatrix;
	data.cameraPosition = &cameraPosition;

	for (int i = 0; i < m_mesh->shapeCount(); i++){
		if (renderPass != nullptr) {
			auto material = m_materials[m_mesh->m_shapes[i].materialIndex];
			renderPass->PrepareMaterial(material);
		}
		m_mesh->DrawShape(data, i);
	}

	///TODO: 
	/// boundMesh.verteces * transMatrix 
	/// � �������� � navmesh 
}

void MeshComponent::OnDrawShape(int index) {
	if (!visible || m_mesh == nullptr)
		return;
	if(!TempVisible && !ingnore_TempVisible)
		return;

	if (m_meshVersion != m_mesh->version)
		m_OnMeshReload();

	auto camera = m_render->camera();
	auto cameraPosition = camera->worldPosition();
	auto worldMatrix = Matrix::CreateScale(meshScale) * GetWorldMatrix();
	auto transMatrix = worldMatrix * camera->cameraMatrix();

	Mesh4::DynamicShapeData data;
	data.render = m_render;
	data.worldMatrix = &worldMatrix;
	data.transfMatrix = &transMatrix;
	data.cameraPosition = &cameraPosition;

	m_mesh->DrawShape(data, index);

	///TODO: 
	/// ���� ������ ��� �� update
	/// boundMesh.verteces * transMatrix 
	/// � �������� � navmesh 
}


DEF_COMPONENT(MeshComponent, Engine.MeshComponent, 2) {
	OFFSET(0, MeshComponent, isDebug);
	OFFSET(1, MeshComponent, visible);
}

DEF_PROP_GET(MeshComponent, bool, IsDynamic)
DEF_PROP_GET(MeshComponent, bool, IsStatic)
DEF_PROP_GET(MeshComponent, int, MaterialCount)

DEF_FUNC(MeshComponent, SetFromCs, void)(CppRef compRef, CppRef meshRef) {
	auto component = CppRefs::ThrowPointer<MeshComponent>(compRef);
	auto mesh = CppRefs::ThrowPointer<Mesh4>(meshRef);

	component->SetMeshFromCs(mesh);
}


DEF_FUNC(MeshComponent, AddShape, void)( 
	CppRef compRef,
	Mesh4::Vertex* verteces, 
	int vlength, 
	int* indeces, 
	int ilength, 
	int matIndex) 
{
	auto component = CppRefs::ThrowPointer<MeshComponent>(compRef);
	component->AddShape(verteces, vlength, indeces, ilength, matIndex);
}

DEF_FUNC(MeshComponent, RemoveMaterials, void)(CppRef compRef) {
	CppRefs::ThrowPointer<MeshComponent>(compRef)->RemoveMaterials();
}

DEF_FUNC(MeshComponent, RemoveMaterial, void)(CppRef compRef, int index) {
	CppRefs::ThrowPointer<MeshComponent>(compRef)->RemoveMaterial(index);
}

DEF_FUNC(MeshComponent, ClearMesh, void)(CppRef compRef) {
	CppRefs::ThrowPointer<MeshComponent>(compRef)->ClearMesh();
}

DEF_FUNC(MeshComponent, SetMaterial, void)(CppRef compRef, size_t index, CppRef materialRef) {
	auto material = CppRefs::ThrowPointer<Material>(materialRef);
	auto meshComp = CppRefs::ThrowPointer<MeshComponent>(compRef);
	
	meshComp->SetMaterial(index, material);
}

DEF_FUNC(MeshComponent, GetMaterial, CppRef)(CppRef compRef, size_t index) {
	auto material = CppRefs::ThrowPointer<MeshComponent>(compRef)->GetMaterial(index);
	return material != nullptr ? CppRefs::GetRef((void*)material) : RefCpp(0);
}

DEF_FUNC(MeshComponent, SetPreInitMesh, void)(CppRef compRef, CppRef meshRef) {
	auto meshComp = CppRefs::ThrowPointer<MeshComponent>(compRef);
	auto mesh = CppRefs::ThrowPointer<Mesh4>(meshRef);

	meshComp->m_preinitMesh = mesh;
}

DEF_FUNC(MeshComponent, SetPreInitMaterials, void)(CppRef compRef, size_t* matRefs, int count) {
	auto* meshComp = CppRefs::ThrowPointer<MeshComponent>(compRef);

	meshComp->m_preinitMaterials.clear();

	auto ptr = matRefs;
	for (int i = 0; i < count; i++, ptr++) {
		auto cppRef = RefCpp(*ptr);
		auto* material = CppRefs::ThrowPointer<Material>(cppRef);

		meshComp->m_preinitMaterials.push_back(material);
	}
}