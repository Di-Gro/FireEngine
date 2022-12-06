#include "OldPass.h"
#include "Render.h"

void OldPass::Draw() {

	m_BeginDraw();

	m_render->m_Draw();
	//m_render->m_DrawUI();

	m_EndDraw();
}