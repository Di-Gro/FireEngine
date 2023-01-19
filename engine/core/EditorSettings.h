#pragma once
#include <string>

#include "CSBridge.h"

class EditorSettings {
public:
	static std::string startupSceneId;
	static int startupSceneIdHash;
};

FUNC(Game, SetEditorSettings, void)(CppRef gameRef, const char* startupScene);