#include "RenderPass.h"
#include "Render.h"


void RenderPass::Init(Render* render) {
	m_render = render;
}

void RenderPass::SetRenderTargets(std::initializer_list<RenderTarget*> targets) {

	for (int i = 0; i < 8; i++) {
		if (i < targets.size()) {
			auto* rt = *(targets.begin() + i);
			m_renderTargets[i] = rt;
			m_dxRenderTargets[i] = rt->renderTarget();
		}
		else {
			m_renderTargets[i] = nullptr;
			m_dxRenderTargets[i] = nullptr;
		}
	}
}

void RenderPass::m_BeginDraw() {
	auto* depthStencil = useDepthStencil ? m_render->depthStencil() : nullptr;

	// Очищаем render target-ы 
	for (int i = 0; i < 8; i++) {
		if (m_renderTargets[i] != nullptr)
			m_renderTargets[i]->Clear();
	}

	// Устанавливаем render target-ы 
	m_render->context()->OMSetRenderTargets(8, m_dxRenderTargets, depthStencil);

	/// TODO: взять rastState из материала
	m_render->context()->RSSetState(m_render->m_device.rastState());
}

//void RenderPass::SetMaterial(const Material*) {
//
//}

void RenderPass::m_EndDraw() {
	
}

void RenderPass::Draw() {
	m_BeginDraw();
	m_EndDraw();
}