#include "EditorSettings.h"
#include "Game.h"
#include "Assets.h"

std::string EditorSettings::startupSceneId = "";
int EditorSettings::startupSceneIdHash = 0;

DEF_FUNC(Game, SetEditorSettings, void)(CppRef gameRef, const char* startupSceneId) {
	auto game = CppRefs::ThrowPointer<Game>(gameRef);

	game->editorSettings.startupSceneId = startupSceneId;
	game->editorSettings.startupSceneIdHash = game->assets()->GetAssetIDHash(startupSceneId);
}