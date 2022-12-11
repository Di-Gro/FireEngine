#pragma once
#include <vector>
#include <string>
#include <list>

#include <d3d11.h>
#include <SimpleMath.h>

#include "Forms.h"
#include "Mesh.h"
#include "Game.h"
#include "MaterialAlias.h"
#include "IShadowCaster.h"

using namespace DirectX::SimpleMath;

class PointLight;
class SpotLight;
class CameraComponent;
class DirectionLight;
class AmbientLight;

class TestLightComponent : public Component {
	PURE_COMPONENT(TestLightComponent)

private:
	DirectionLight* m_directionalLight = nullptr;
	AmbientLight* m_ambientLight = nullptr;
	PointLight* m_pointLight = nullptr;
	SpotLight* m_spotLight = nullptr;
	
	CameraComponent* m_pointCamera = nullptr;
	CameraComponent* m_spotCamera = nullptr;

	CameraComponent* m_defaultCamera;

	bool m_isOpened = true;

public:
	void OnInit() override;
	void OnUpdate() override;

private:
	void m_DrawGUI();
};

