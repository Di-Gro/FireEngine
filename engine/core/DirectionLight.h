#pragma once

#include "Actor.h"
#include "MaterialAlias.h"
#include "ILightSource.h"
#include "CSBridge.h"
#include "ScreenQuad.h"

#include "ShaderResource.h"
#include "TextureResource.h"
#include "RenderTarget.h"
#include "DepthStencil.h"

class MeshComponent;
class LineComponent;
class CameraComponent;

class DirectionLight : public Component, public ILightSource {
	COMPONENT(DirectionLight);

public:
	Vector3 color = Vector3::One;
	float intensity = 0.6f;

private:
	TextureResource m_renderTexture;
	RenderTarget m_renderTarget;

	TextureResource m_depthTexture;
	DepthStencil m_depthStencil;
	ShaderResource m_depthResource;

	CameraComponent* m_camera = nullptr;

	MeshComponent* m_debugMesh = nullptr;
	LineComponent* m_debugLine = nullptr;

	float m_mapScale = 1;
	Matrix m_uvMatrix;

	bool m_needDrawDebug = true;

	ScreenQuad m_screenQuad;

	Pass::LightSource m_lightSource;

public:

	inline float mapScale() { return m_mapScale; }

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
	void OnDestroy() override;

	void OnDrawLight(RenderPass* renderPass) override;
	LightCBuffer GetCBuffer() override;

	void Resize(float width, float height);

	void RecieveGameMessage(const std::string& msg);
};

DEC_COMPONENT(DirectionLight);