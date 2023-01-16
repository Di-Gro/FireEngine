#include "CameraComponent.h"
#include "Game.h"
#include "Scene.h"
#include "Render.h"

void CameraComponent::Attach() { 
	scene()->mainCamera(this);
}

void CameraComponent::Deattach() {
	if (IsAttached())
		scene()->mainCamera(nullptr);
}

void CameraComponent::OnInit() {
	m_cameraIter = scene()->AddCamera(this);
}

void CameraComponent::OnDestroy() {
	scene()->RemoveCamera(m_cameraIter);
	Deattach();
}

bool CameraComponent::IsAttached() { 
	return scene()->mainCamera() == this;
}

bool CameraComponent::IsMainCamera() { 
	return game()->mainCamera() == this;
}

void CameraComponent::UpdateProjectionMatrix() {
	auto size = scene()->renderer.viewportSize();

	if (m_useOrthographic) {
		m_projMatrix = Matrix::CreateOrthographic(orthoWidth, orthoHeight, orthoNearPlane, orthoFarPlane);
	}
	else {
		m_projMatrix = Matrix::CreatePerspectiveFieldOfView(
			3.14f * 0.35f,
			size.x / size.y,
			nearPlane,
			farPlane);
	}
}

DEF_COMPONENT(CameraComponent, Engine.CameraComponent, 8, RunMode::EditPlay) {
	OFFSET(0, CameraComponent, orthoWidth);
	OFFSET(1, CameraComponent, orthoHeight);
	OFFSET(2, CameraComponent, orthoNearPlane);
	OFFSET(3, CameraComponent, orthoFarPlane);
	OFFSET(4, CameraComponent, nearPlane);
	OFFSET(5, CameraComponent, farPlane);
	OFFSET(6, CameraComponent, drawDebug);
	OFFSET(7, CameraComponent, isPlayerCamera);
}

DEF_PROP_GET(CameraComponent, bool, IsAttached)
DEF_PROP_GETSET(CameraComponent, bool, orthographic)
DEF_PROP_GETSET(CameraComponent, Matrix, viewMatrix)

DEF_FUNC(CameraComponent, Attach, void)(CppRef compRef) {
	CppRefs::ThrowPointer<CameraComponent>(compRef)->Attach();
}

DEF_FUNC(CameraComponent, UpdateProjMatrix, void)(CppRef compRef) {
	CppRefs::ThrowPointer<CameraComponent>(compRef)->UpdateProjectionMatrix();
}