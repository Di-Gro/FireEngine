#pragma once

#include "Game.h"
#include "ShaderResource.h"

class CameraComponent;
class Player;
class ShadowPass;

class RenderPassUI : public Component {
	PURE_COMPONENT(RenderPassUI)

private:
	ShaderResource resourceDs;

public:
	void OnStart() override;
	void OnUpdate() override;

};
