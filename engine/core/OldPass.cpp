#include "OldPass.h"
#include "Render.h"
#include "RenderDevice.h"

void OldPass::Draw() {

	BeginDraw();
	m_render->context()->RSSetState(m_render->GetRastState(CullMode::Back));
	 
	m_render->m_Draw();
	//m_render->m_DrawUI();

	EndDraw();
}