#include "TestLightComponent.h"

#include "imgui\imgui.h"

#include "PointLight.h"
#include "SpotLight.h"
#include "FlyingCamera.h"
#include "AmbientLight.h"
#include "DirectionLight.h"

DEF_PURE_COMPONENT(TestLightComponent)

void TestLightComponent::OnInit() {
	m_defaultCamera = game()->mainCamera();

	m_directionalLight = game()->lighting()->directionLight();
	m_ambientLight = AddComponent<AmbientLight>();
	
}

void TestLightComponent::OnUpdate() {
	auto input = game()->hotkeys();
	if (input->GetButtonDown(Keys::L) && input->GetButton(Keys::Ctrl)) {
		m_isOpened = true;
		game()->inFocus = false;
	}

	if (m_isOpened && !game()->inFocus) {
		m_DrawGUI();
	}
}

void TestLightComponent::m_DrawGUI() {
	ImGui::Begin("Test Lights", &m_isOpened);

	bool isSpot = game()->mainCamera() == m_spotCamera;
	bool isPoint = game()->mainCamera() == m_pointCamera;

	/// Directional
	if (m_directionalLight != nullptr) {
		ImGui::Text("Directional");
		ImGui::SliderFloat("D.Intencity", &m_directionalLight->intensity, 0, 1);
	}

	/// Ambient
	bool checkboxValue = m_ambientLight != nullptr;
	if (ImGui::Checkbox("Ambient Light", &checkboxValue)) {
		if (checkboxValue) {
			m_ambientLight = AddComponent<AmbientLight>();
		}
		else {
			m_ambientLight->Destroy();
			m_ambientLight = nullptr;
		}
	}
	if (m_ambientLight != nullptr) {
		ImGui::SameLine();
		ImGui::Text("Ambient");
		ImGui::SliderFloat("A.Intencity", &m_ambientLight->intensity, 0, 1);
	}

	/// Point
	checkboxValue = m_pointLight != nullptr;
	if (ImGui::Checkbox("Point Light", &checkboxValue)) {
		if (checkboxValue) {
			m_pointLight = CreateActor("PointLight")->AddComponent<PointLight>();
			m_pointCamera = m_pointLight->AddComponent<FlyingCamera>();
		}
		else {
			m_pointLight->actor()->Destroy();
			m_pointLight = nullptr;
		}
	}
	if (m_pointLight != nullptr) {
		ImGui::SameLine();
		if (ImGui::RadioButton("", isPoint)) {
			if (isPoint)
				m_defaultCamera->Attach();
			else
				m_pointCamera->Attach();
		}
		ImGui::SameLine();
		ImGui::Text("Point");
		ImGui::SliderFloat("P.Intencity", &m_pointLight->intensity, 0, 3);
		ImGui::SliderFloat("P.Radius", &m_pointLight->radius, 0, 400);
	}

	/// Spot
	checkboxValue = m_spotLight != nullptr;
	if (ImGui::Checkbox("Spot Light", &checkboxValue)) {
		if (checkboxValue) {
			m_spotLight = CreateActor("SpotLight")->AddComponent<SpotLight>();
			m_spotCamera = m_spotLight->AddComponent<FlyingCamera>();
		}
		else {
			m_spotLight->actor()->Destroy();
			m_spotLight = nullptr;
		}
	}
	if (m_spotLight != nullptr) {
		ImGui::SameLine();
		if (ImGui::RadioButton("", isSpot)) {
			if (isSpot)
				m_defaultCamera->Attach();
			else
				m_spotCamera->Attach();
		}
		ImGui::SameLine();
		ImGui::Text("Spot");
		ImGui::SliderFloat("S.Intencity", &m_spotLight->intensity, 0, 1);
		ImGui::SliderFloat("S.Angle", &m_spotLight->angle, 0, 180);
		ImGui::SliderFloat("S.Blend", &m_spotLight->blend, 0, 1);
		ImGui::SliderFloat("S.Length", &m_spotLight->length, 0, 2000);
		ImGui::SliderFloat("S.Attenuation", &m_spotLight->attenuation, 0, 5);
	}
	ImGui::End();
}