#pragma once

#include "Game.h"

class TransformTest : public Component {
public:
	//TransformTest(GameObject* go) : Component(go) {}

	void OnUpdate() override;
	
private:

	void m_TestLocalPosition();
	void m_TestLocalPosition2();

};

