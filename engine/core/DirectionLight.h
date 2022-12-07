#pragma once

#include "Game.h"
#include "RenderTarget.h"
#include "DepthStencil.h"

class MeshComponent;
class LineComponent;
class CameraComponent;

class DirectionLight : public Component {
	PURE_COMPONENT(DirectionLight);
public:
	Vector3 color = Vector3::One;
	float intensity = 1.0f;

private:
	Texture m_renderTexture;
	RenderTarget m_renderTarget;

	Texture m_depthTexture;
	DepthStencil m_depthStencil;
	ShaderResource m_depthResource;

	CameraComponent* m_camera = nullptr;

	MeshComponent* m_debugMesh = nullptr;
	LineComponent* m_debugLine = nullptr;

	Matrix m_uvMatrix;

	bool m_needDrawDebug = true;

public:
	RenderTarget* RT() { { return &m_renderTarget; } };
	DepthStencil* DS() { { return &m_depthStencil; } };

	ID3D11RenderTargetView* renderTarget() { return m_renderTarget.get(); }

	ID3D11Texture2D* depthTexture() { return m_depthTexture.get(); }
	ID3D11DepthStencilView* depthStencil() { return m_depthStencil.get(); }
	ShaderResource* depthResource() { return &m_depthResource; }

	bool drawDebug() { return m_needDrawDebug; }
	void drawDebug(bool value);

	CameraComponent* camera();
	Matrix uvMatrix() { return m_uvMatrix; }

	void OnInit() override;

	void RecieveGameMessage(const std::string& msg);
};

