#include "CameraComponent.h"

void CameraComponent::Attach() { 
	game()->mainCamera(this); 
}

bool CameraComponent::IsAttached() { 
	return game()->mainCamera() == this; 
}

void CameraComponent::UpdateProjectionMatrix(/*Window* window*/) {
	auto window = game()->window();

	if (!m_useOrthographic) {
		m_projMatrix = Matrix::CreatePerspectiveFieldOfView(
			3.14f * 0.35f,
			window->GetWidth() / (float)window->GetHeight(),
			nearPlane,
			farPlane);
	}
	else {
		m_projMatrix = Matrix::CreateOrthographic(orthoWidth, orthoHeight, orthoNearPlane, orthoFarPlane);
	}
}

DEF_COMPONENT(CameraComponent, Engine.CameraComponent, 7) { 
	OFFSET(0, CameraComponent, orthoWidth);
	OFFSET(1, CameraComponent, orthoHeight);
	OFFSET(2, CameraComponent, orthoNearPlane);
	OFFSET(3, CameraComponent, orthoFarPlane);
	OFFSET(4, CameraComponent, nearPlane);
	OFFSET(5, CameraComponent, farPlane);
	OFFSET(6, CameraComponent, drawDebug);
}

DEF_PROP_GET(CameraComponent, bool, IsAttached)
DEF_PROP_GETSET(CameraComponent, bool, orthographic)

DEF_FUNC(CameraComponent, Attach, void)(CppRef compRef) {
	CppRefs::ThrowPointer<CameraComponent>(compRef)->Attach();
}

DEF_FUNC(CameraComponent, UpdateProjMatrix, void)(CppRef compRef) {
	CppRefs::ThrowPointer<CameraComponent>(compRef)->UpdateProjectionMatrix();
}