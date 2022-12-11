#pragma once

class RenderPass;
class Component;

class IShadowCaster {
public:
	virtual void OnDrawShadow(RenderPass* renderPass) = 0;
	virtual Component* GetComponent() = 0;
};