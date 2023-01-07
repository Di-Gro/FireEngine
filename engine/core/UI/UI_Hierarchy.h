#pragma once

#include "../Texture.h"
#include "../ShaderResource.h"
#include "../ImageAsset.h"
#include "../SimpleMath.h"
#include "../imgui/imgui.h"

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
	void Draw_UI_Hierarchy();
	void Init(Game* game);
	void VisitActor(Actor* actor, int index, std::list<Actor*>::iterator rootIter);
	void DrawTreeNode(Actor* actor);
	void HandleDrag(Actor* actor);
	void HandleDrop(Actor* actor, bool selectedTree, float height, ImVec2 size, ImVec2 cursor);
	void HandeDragDrop(Actor* drag, Actor* drop, bool isDropOpen, float height);

private:
	bool m_FindTargetInActorParent(Actor* actor, Actor* target);
	void m_InitIcons();
	void m_DrawItemSeparator(ShaderResource* icon, bool isBeforeItem, ImVec2 size, ImVec2 cursor);

private:
	Game* m_game;
	UserInterface* m_ui;
	ReorderEnum m_reorderEnum;

	Image m_icMoveSeparator;
	Texture m_moveSeparatorTex;
	ShaderResource m_moveSeparatorRes;

	bool m_isMouseReleaseOnDragActor = false;

	Actor* m_dragTargetActor = nullptr;
};