#pragma once

#include "CSBridge.h"
#include "CSLinked.h"

class Actor;
class Game;


FUNC(Game, CreateGameObjectFromCS, GameObjectInfo)(CppRef gameRef, CsRef csRef, CppRef parentRef);
FUNC(Game, GetRootActorsCount, int)(CppRef gameRef);
FUNC(Game, WriteRootActorsRefs, void)(CppRef gameRef, CsRef* refs);

class Scene {
	FRIEND_FUNC(Game, CreateGameObjectFromCS, GameObjectInfo)(CppRef gameRef, CsRef csRef, CppRef parentRef);

	friend class Game;

private:
	static unsigned int m_objectCount;
	static mono::mono_method_invoker<CppRef()> mono_create;
	static bool mono_inited;

public:
private:
	Game* m_game;
	std::list<Actor*> m_actors;

public:
	void Init(Game* game);
	void MoveActor(Actor* from, Actor* to, bool isPastBefore);

	Actor* CreateActor(std::string name = "") { return CreateActor(nullptr, name); }
	Actor* CreateActor(Actor* parent, std::string name = "");

	void DestroyActor(Actor* actor);

	int GetRootActorsCount();
	void WriteRootActorsRefs(CsRef* refs);

	std::list<Actor*>::iterator GetNextRootActors(const std::list<Actor*>::iterator& iter);
	std::list<Actor*>::iterator BeginActor() { return m_actors.begin(); }
	std::list<Actor*>::iterator EndActor() { return m_actors.end(); }

private:
	void f_Update();
	void f_Destroy();
	void f_RemoveActor(Actor* actor);
	void f_AddActor(Actor* actor);

private:
	void m_InitMono();

	GameObjectInfo m_CreateActorFromCs(CsRef csRef, CppRef parentRef);

	std::list<Actor*>::iterator m_EraseActor(std::list<Actor*>::iterator it);

};