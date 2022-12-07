#include "OldPass.h"
#include "Render.h"

void OldPass::Draw() {

	BeginDraw();

	m_render->m_Draw();
	//m_render->m_DrawUI();

	EndDraw();
}