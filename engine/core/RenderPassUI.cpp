#include "RenderPassUI.h"

#include "imgui\imgui.h"

DEF_PURE_COMPONENT(RenderPassUI)


void RenderPassUI::OnStart() {
	auto opaquePass = game()->render()->opaquePass();

	//resourceDs = ShaderResource::Create(game()->render()->depthRes());

	//resource0 = ShaderResource::Create(&opaquePass->target0Tex);
	//resource1 = ShaderResource::Create(&opaquePass->target1Tex);
	//resource2 = ShaderResource::Create(&opaquePass->target2Tex);
	//resource3 = ShaderResource::Create(&opaquePass->target3Tex);
}

void RenderPassUI::OnUpdate() {
	//if (game()->hotkeys()->GetButton(Keys::Tab)) {
		
	auto opaquePass = game()->render()->opaquePass();

	ImGui::Begin("target0Tex: diffuse, spec");
	ImGui::Image(opaquePass->target0Res.get(), { 1920 / 6, 1061 / 6 });
	ImGui::End();
		
	ImGui::Begin("target1Tex: normals");
	ImGui::Image(opaquePass->target1Res.get(), { 1920 / 6, 1061 / 6 });
	ImGui::End();

	ImGui::Begin("target2Tex: vertex color");
	ImGui::Image(opaquePass->target2Res.get(), { 1920 / 6, 1061 / 6 });
	ImGui::End();

	ImGui::Begin("target3Tex: world pos");
	ImGui::Image(opaquePass->target3Res.get(), { 1920 / 6, 1061 / 6 });
	ImGui::End();

	ImGui::Begin("targetDs: Depth");
	ImGui::Image(game()->render()->depthRes()->get(), { 1920 / 6, 1061 / 6 });
	ImGui::End();

		//ImGui::Begin("targetDs: Depth");
		//ImGui::Image(game()->render()->rdevice()->depthSRV, { 1920 / 6, 1061 / 6 });
		//ImGui::End();
	//}
}