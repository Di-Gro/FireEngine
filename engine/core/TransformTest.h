#pragma once

#include "Actor.h"

class TransformTest : public Component {
public:
	//TransformTest(Actor* go) : Component(go) {}

	void OnUpdate() override;
	
private:

	void m_TestLocalPosition();
	void m_TestLocalPosition2();

};

