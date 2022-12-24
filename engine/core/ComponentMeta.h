#pragma once

#include <mono_game_types.h>

class ClassInfo;


class ComponentMeta {
public:
	bool isPure;
	const char* name;
	ClassInfo* classInfo = nullptr;
	CppRef classInfoRef;
};