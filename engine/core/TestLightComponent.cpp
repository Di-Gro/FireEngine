#include "TestLightComponent.h"

#include "imgui\imgui.h"

#include "Game.h"
#include "Lighting.h"
#include "HotKeys.h"
#include "UI/UserInterface.h"

#include "PointLight.h"
#include "SpotLight.h"
#include "FlyingCamera.h"
#include "AmbientLight.h"
#include "DirectionLight.h"

DEF_PURE_COMPONENT(TestLightComponent, RunMode::EditOnly)

void TestLightComponent::OnInit() {
	m_directionalLight = GetComponent<DirectionLight>();
	m_ambientLight = GetComponent<AmbientLight>();
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

	auto selectedActor = game()->ui()->GetActor();

	bool isSpot = selectedActor != nullptr && m_spotLight != nullptr && selectedActor == m_spotLight->actor();
	bool isPoint = selectedActor != nullptr && m_pointLight != nullptr && selectedActor == m_pointLight->actor();

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
		}
		else {
			m_pointLight->actor()->Destroy();
			m_pointLight = nullptr;
		}
	}
	if (m_pointLight != nullptr) {
		ImGui::SameLine();
		if (ImGui::RadioButton("##Select Point Light", isPoint)) {
			if (isPoint)
				game()->ui()->SelectedActor(nullptr);
			else
				game()->ui()->SelectedActor(m_pointLight->actor());
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
		}
		else {
			m_spotLight->actor()->Destroy();
			m_spotLight = nullptr;
		}
	}
	if (m_spotLight != nullptr) {
		ImGui::SameLine();
		if (ImGui::RadioButton("##Select Spot Light", isSpot)) {
			if (isSpot)
				game()->ui()->SelectedActor(nullptr);
			else
				game()->ui()->SelectedActor(m_spotLight->actor());
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