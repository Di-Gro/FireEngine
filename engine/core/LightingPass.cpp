#include "LightingPass.h"

#include "Render.h"
#include "CameraComponent.h"
#include "MaterialAlias.h"
#include "Mesh.h"
#include "MeshComponent.h"
#include "DirectionLight.h"

#include "RenderTarget.h"
#include "ShaderResource.h"
#include "DepthStencil.h"


void LightingPass::Init(Game* game) {
	RenderPass::Init(game);

	blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;

	UpdateBlendState();
}

void LightingPass::Draw() {

}