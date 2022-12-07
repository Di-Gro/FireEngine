#pragma once
#include "RenderPass.h"

class LightingPass : public RenderPass {

public:
	void Init(Game* game) override;
	void Draw() override;

};

