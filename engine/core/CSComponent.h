#pragma once

#include "Game.h"

/// <summary>
/// C++ тень C# компонента.
/// </summary>
class CsComponent : public Component {
	COMPONENT(CsComponent)
};
DEC_COMPONENT(CsComponent);


///// <summary>
///// C++ компонент.
///// </summary>
//class CppComponent : public Component {
//	COMPONENT(CppComponent)
//
//public:
//	void OnDestroy() override {
//		//std::cout << "+: CppComponent(" << csRef() << ",  " << cppRef() << ").OnDestroy()" << std::endl;
//	}
//
//};
//DEC_COMPONENT(CppComponent);