#include "EditorSettings.h"
#include "Game.h"

std::string EditorSettings::startupScene = "";

DEF_FUNC(Game, SetEditorSettings, void)(CppRef gameRef, const char* startupScene) {
	auto game = CppRefs::ThrowPointer<Game>(gameRef);

	game->editorSettings.startupScene = startupScene;
}