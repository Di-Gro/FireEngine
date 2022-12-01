#include "MeshComponent.h"

#include <SimpleMath.h>

#include "Game.h"
#include "CameraComponent.h"
#include "LineComponent.h"

using namespace DirectX::SimpleMath;


bool MeshComponent::mono_inited = false;
mono::mono_method_invoker<void(CsRef, CppRef)> MeshComponent::mono_SetFromCpp;


void MeshComponent::OnInit() {
	m_InitMono();
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

	auto* newMesh = new Mesh4(*m_mesh);
	newMesh->f_ref = CppRefs::Create(newMesh);
	newMesh->f_cppRef = newMesh->f_ref.cppRef();
		
	mesh(newMesh);
	m_dynamicMesh = newMesh;
}

void MeshComponent::OnDestroy() {
	m_DeleteResources();
}

void MeshComponent::RemoveMaterial(size_t index) {
	if (index > m_mesh->maxMaterialIndex())
		return;

	// ������� ������ ��������
	m_DeleteLocalDynamicMaterial(index);

	// ������ ����������� ��������
	auto defaultMaterial = game()->meshAsset()->GetStaticMaterial(MeshAsset::defaultMaterialName);
	m_materials[index] = defaultMaterial;
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
	if (IsDynamic()) {
		CppRefs::Remove(m_dynamicMesh->f_ref);
		delete m_dynamicMesh;
		m_dynamicMesh = nullptr;
		m_mesh = nullptr;
	}
	m_DeleteMaterials();
}

void MeshComponent::m_DeleteMaterials() {
	for (int i = 0; i < m_dynamicMaterials.size(); i++)
		m_DeleteLocalDynamicMaterial(i);

	m_materials.clear();
	m_dynamicMaterials.clear();
}

void MeshComponent::m_DeleteLocalDynamicMaterial(int index) {
	auto localMaterial = m_dynamicMaterials[index];
	if (localMaterial == nullptr)
		return;

	game()->meshAsset()->DeleteDynamicMaterial(localMaterial);

	m_dynamicMaterials[index] = nullptr;
}

void MeshComponent::AddShape(
	std::vector<Mesh4::Vertex>* verteces, 
	std::vector<int>* indeces, 
	size_t materialIndex) 
{
	m_InitDynamic();

	m_dynamicMesh->AddShape(verteces, indeces, game()->render(), materialIndex);

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

	m_dynamicMesh->AddShape(verteces, vertecesLength, indeces, indecesLength, game()->render(), materialIndex);

	m_FillByDefaultMaterial(m_dynamicMesh->maxMaterialIndex() + 1);
}

void MeshComponent::SetMaterial(size_t index, const fs::path& shaderPath) {
	m_InitDynamic();

	if (index > m_mesh->maxMaterialIndex())
		return;

	assert(index < m_dynamicMaterials.size());

	// ������� ��������� ��������
	m_DeleteLocalDynamicMaterial(index);

	// ������� ����� ��������
	auto material = game()->meshAsset()->CreateDynamicMaterial("Generated by MeshComponent", shaderPath);

	// ���������� ����� ��������
	m_materials[index] = material;
	m_dynamicMaterials[index] = material;
}

void MeshComponent::SetMaterial(size_t index, const Material* other) {
	if (index > m_mesh->maxMaterialIndex())
		return;

	assert(index < m_dynamicMaterials.size());

	// ������� ��������� ��������
	m_DeleteLocalDynamicMaterial(index);

	// ���������� ����� ��������
	m_materials[index] = other;
	m_dynamicMaterials[index] = nullptr;
}

const Material* MeshComponent::GetMaterial(size_t index) {
	if (index >= m_materials.size())
		return nullptr;

	return m_materials[index];
}

void MeshComponent::m_FillByDefaultMaterial(int targetSize) {
	if (m_materials.size() < targetSize) {

		auto defaultMaterial = game()->meshAsset()->GetStaticMaterial(MeshAsset::defaultMaterialName);

		for (int i = m_materials.size(); i < targetSize; i++) {
			m_materials.push_back(defaultMaterial);
			m_dynamicMaterials.push_back(nullptr);
		}
	}
}

void MeshComponent::mesh(const Mesh4* mesh) {
	m_DeleteResources();
	m_mesh = mesh;
	m_SetMaterialsFromMesh();

	if (csRef() > 0) {
		auto newRef = m_mesh != nullptr ? m_mesh->f_cppRef : RefCpp(0);
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

	auto meshAsset = game()->meshAsset();

	// ����� ������ ��������� ����
	auto* staticMaterials = meshAsset->GetMaterials(m_mesh);
	auto newSize = staticMaterials != nullptr ? staticMaterials->size() : 0;

	// ���������� ������ ��������� 
	m_materials.reserve(newSize);
	m_dynamicMaterials.reserve(newSize);
	 
	for (int i = 0; i < newSize; i++) {
		m_materials.push_back(staticMaterials->at(i));
		m_dynamicMaterials.push_back(nullptr);
	}

	// ���� ����� ���������� ������, ��� ������������,
	// ��������� ������������ �����������.
	m_FillByDefaultMaterial(m_mesh->maxMaterialIndex() + 1);
}

void MeshComponent::OnDraw() {
	if (!isDebug)
		m_Draw();
}

void MeshComponent::OnDrawDebug() {
	if (isDebug)
		m_Draw();
}

void MeshComponent::m_Draw() {
	if (!visible || m_mesh == nullptr)
		return;

	auto camera = game()->render()->camera();
	auto cameraPosition = camera->transform->worldPosition();
	auto worldMatrix = transform->GetWorldMatrix(); // *Matrix::CreateScale(100, 100, 100);
	auto transMatrix = worldMatrix * camera->cameraMatrix();

	Mesh4::DynamicData data;
	data.render = game()->render();
	data.materials = &m_materials;
	data.worldMatrix = &worldMatrix;
	data.transfMatrix = &transMatrix;
	data.directionLight = game()->lighting()->directionLight();
	data.cameraPosition = &cameraPosition;

	m_mesh->Draw(data);
}


DEF_COMPONENT(MeshComponent, Engine.MeshComponent, 2) {
	OFFSET(0, MeshComponent, isDebug);
	OFFSET(1, MeshComponent, visible);
}

DEF_PROP_GET(MeshComponent, bool, IsDynamic)
DEF_PROP_GET(MeshComponent, bool, IsStatic)
DEF_PROP_GET(MeshComponent, bool, MaterialCount)

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
	return material != nullptr ? material->f_cppRef : RefCpp(0);
}
