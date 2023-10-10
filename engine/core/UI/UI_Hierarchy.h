#pragma once

#include "../Texture.h"
#include "../ShaderResource.h"
#include "../ImageAsset.h"
#include "../SimpleMath.h"
#include "../imgui/imgui.h"

#include "ScenePickerPopup.h"

#include <list>

class Game;
class Actor;
class UserInterface;

enum class ReorderEnum
{
	SELF,
	BETWEEN
};

class UI_Hierarchy
{
public:
	static const char* ActorDragType;

	static void PushPopupStyles();
	static void PopPopupStyles();

private:
	Game* m_game;
	UserInterface* m_ui;

	Image m_icMoveSeparator;
	Texture m_moveSeparatorTex;
	ShaderResource m_moveSeparatorRes;

	bool m_isMouseReleaseOnDragActor = false;
	Actor* m_dragTargetActor = nullptr;
	Actor* m_clickedActor = nullptr;

	ScenePickerPopup m_scenePickerPopup;

public:
	void Draw_UI_Hierarchy();
	void Init(Game* game);
	void VisitActor(Actor* actor, int index, std::list<Actor*>::iterator rootIter);
	void DrawTreeNode(Actor* actor);
	void HandleDrag(Actor* actor);
	void HandleDrop(Actor* actor, bool selectedTree, float height, ImVec2 size, ImVec2 cursor);
	bool HandeDragDrop(Actor* drag, Actor* drop, bool isDropOpen, float height);

private:
	void m_InitIcons();

	void m_DrawSceneContextMenu();
	void m_DrawActorContextMenu(Actor* actor);
	void m_DrawItemSeparator(ShaderResource* icon, bool isBeforeItem, ImVec2 size, ImVec2 cursor);
	
	bool m_FindTargetInActorParent(Actor* actor, Actor* target);

	void m_HandleInput();
	void m_DrawSceneHeader();
	
};
