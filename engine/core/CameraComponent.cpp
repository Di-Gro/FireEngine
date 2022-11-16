#include "CameraComponent.h"

void CameraComponent::Attach() { 
	game()->mainCamera(this); 
}

bool CameraComponent::IsAttached() { 
	return game()->mainCamera() == this; 
}

void CameraComponent::UpdateProjectionMatrix(Window* window) {

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