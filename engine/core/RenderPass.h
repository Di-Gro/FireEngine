#pragma once
#include <initializer_list>

#include <directxmath.h>
#include <d3d11.h>

#include "wrl.h_d3d11_alias.h"
#include "RenderTarget.h"
#include "MaterialAlias.h"

class Render;

class RenderPass {
	friend class Render;

protected: /// For class friends

	Pass::SortedMaterials f_sortedMaterials;
	void f_name(const std::string& value) { m_name = value; }
			
public:
	bool useDepthStencil = true;

protected:
	Render* m_render;
		
private:
	RenderTarget* m_renderTargets[8] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
	ID3D11RenderTargetView* m_dxRenderTargets[8] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

	std::string m_name;

public:
	const std::string& name() { return m_name; }

	void SetRenderTargets(std::initializer_list<RenderTarget*> targets);
	void SetRenderTargets(std::initializer_list<RenderTarget*> targets);

	virtual void Init(Render* render);
	virtual void Draw();

	void m_BeginDraw();
	void m_EndDraw();

};

