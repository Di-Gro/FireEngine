#pragma once

#include "RenderTarget.h"
#include "RenderDevice.h"

class Game;
class GameObject;
class Window;
class CameraComponent;
class ImageComponent;

class Render {
private:
	Game* m_game;
	RenderDevice m_device;
	RenderTarget m_mainRT;

	ImageComponent* m_screenQuad;

	CameraComponent* m_camera = nullptr;

public:
	inline CameraComponent* camera() { return m_camera; }

	inline ID3D11Device* device() { return m_device.GetDevice(); };
	inline ID3D11DeviceContext* context() { return m_device.GetContext(); }
	inline IDXGISwapChain* swapChain() { return m_device.GetSwapChain(); }
	inline comptr<ID3D11RenderTargetView> rtv() { return m_device.GetRTV(); }

	void Init(Game* game, Window* window);
	void Start();
	void Draw();

	void Clear();
	void PrepareFrame();
	void EndFrame();

	void CreateTexture(const ImageAsset::Image* image, Material::Texture& texture, bool useSRGB, bool generateMips);

private:
	void m_Draw(GameObject*);
	void m_DrawUI(GameObject*);

	void m_ForEachGameObject(void (Render::* func) (GameObject*));

};

