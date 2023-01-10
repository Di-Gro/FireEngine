#pragma once

#include <string>

#include "../imgui/imgui.h"

class Game;
class UserInterface;

class UI_AssetsStore
{
public:
	//CsRef csRef;
	void Init(Game* game);
	void DrawAssetsStore();

private:
	Game* m_game;
	std::string m_input;
	int m_popupOpenAssetId;

	bool IsMatch(std::string source, std::string target);
	void m_AssetsStoreContextMenu(int assetId);

	void m_sendAssetIdForCreate(int assetID);
	void m_sendAssetIdForRename(int assetID, std::string newName);
	void m_sendAssetIdForRemove(int assetID);
};