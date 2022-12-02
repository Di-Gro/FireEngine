#pragma once

#include <list>

#include "RenderTarget.h"
#include "RenderDevice.h"

class Game;
class Actor;
class Window;
class CameraComponent;
class ImageComponent;
class Component;

class Render {
private:
	Game* m_game;
	RenderDevice m_device;
	RenderTarget m_mainRT;

	ImageComponent* m_screenQuad;

	CameraComponent* m_camera = nullptr;

	std::list<Component*> m_drawers;
	std::list<Component*> m_uiDrawers;

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

	std::list<Component*>::iterator SubscribeForDrawin(Component* gameObject);
	void UnSubscribeFromDrawin(std::list<Component*>::iterator handle);

	std::list<Component*>::iterator SubscribeForUIDrawin(Component* gameObject);
	void UnSubscribeFromUIDrawin(std::list<Component*>::iterator handle);

	void CreateTexture(const ImageAsset::Image* image, Material::Texture& texture, bool useSRGB, bool generateMips);

private:
	void m_Draw();
	void m_DrawUI();

	//void m_Draw(Actor*);
	//void m_DrawUI(Actor*);

	//void m_ForEachGameObject(void (Render::* func) (Actor*));

};

