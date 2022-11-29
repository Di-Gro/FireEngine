#pragma once

#include "Game.h"

/// <summary>
/// C++ тень C# компонента.
/// </summary>
class CsComponent : public Component {
	OBJECT;

	void OnInit() override {
		//std::cout << "+: CsComponent(" << csRef() << ",  " << cppRef() << ").OnInit()" << std::endl;
	}

	void OnStart() override {
		//std::cout << "+: CsComponent(" << csRef() << ",  " << cppRef() << ").OnStart()" << std::endl;
	}

	void OnUpdate() override {
		//std::cout << "+: CsComponent(" << csRef() << ",  " << cppRef() << ").OnUpdate()" << std::endl;
	}

	void OnDestroy() override {
		//std::cout << "+: CsComponent(" << csRef() << ",  " << cppRef() << ").OnDestroy()" << std::endl;
	}
};
DEC_COMPONENT_CREATE(CsComponent);


/// <summary>
/// C++ компонент.
/// </summary>
class CppComponent : public Component {
	COMPONENT;

public:
	void OnDestroy() override {
		//std::cout << "+: CppComponent(" << csRef() << ",  " << cppRef() << ").OnDestroy()" << std::endl;
	}

};
DEC_COMPONENT(CppComponent);