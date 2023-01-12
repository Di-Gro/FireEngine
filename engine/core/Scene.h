#pragma once

#include "CSBridge.h"
#include "CSLinked.h"
#include "Actor.h"
#include "SceneRenderer.h"
#include "Refs.h"
#include "SafeDestroy.h"

class Game;
class DirectionLight;
class AmbientLight;
class LinedPlain;
class CameraComponent;
class EditorCamera;
class PhysicsScene;

using CameraIter = std::list<CameraComponent*>::iterator;
using SceneIter = std::list<Scene*>::iterator;

FUNC(Game, CreateGameObjectFromCS, GameObjectInfo)(CppRef gameRef, CsRef csRef, CppRef parentRef);
FUNC(Game, GetRootActorsCount, int)(CppRef gameRef);
FUNC(Game, WriteRootActorsRefs, void)(CppRef gameRef, CsRef* refs);

class Scene : public IAsset, public SafeDestroy {
	FRIEND_FUNC(Game, CreateGameObjectFromCS, GameObjectInfo)(CppRef gameRef, CsRef csRef, CppRef parentRef);

	friend class Game;

private:
	static unsigned int m_objectCount;
	static mono::mono_method_invoker<CppRef()> mono_create;
	static bool mono_inited;

public:
	DirectionLight* directionLight = nullptr;
	AmbientLight* ambientLight = nullptr;

	EditorCamera* editorCamera;
	LinedPlain* linedPlain;

	SceneRenderer renderer;
	PhysicsScene* m_physicsScene;

private: // friend
	SceneIter f_sceneIter;
	Ref2 f_ref;
	bool f_isDestroyed = false;

private:
	Game* m_game;
	std::string m_name = "";

	std::list<Actor*> m_actors;
	std::list<Actor*> m_staticActors;
	std::list<CameraComponent*> m_cameras;

	CameraComponent* m_mainCamera = nullptr;

	bool m_isEditor = false;
	bool m_isStarted = false;

public:
	void Init(Game* game, bool _isEditor);
	void Start();
	void Release() override;

	bool IsAsset() { return assetIdHash() != 0; }

	Game* game() { return m_game; }

	const std::string& name() { return m_name; }
	void name(const std::string& value) { m_name = value; }

	inline bool isEditor() { return m_isEditor; }

	void MoveActor(Actor* from, Actor* to, bool isPastBefore);

	Actor* CreateActor(std::string name = "") { return CreateActor(nullptr, name); }
	Actor* CreateActor(Actor* parent, std::string name = "");

	void DestroyActor(Actor* actor);

	int GetRootActorsCount();
	void WriteRootActorsRefs(CsRef* refs);

	std::list<Actor*>::iterator GetNextRootActors(const std::list<Actor*>::iterator& iter);
	std::list<Actor*>::iterator BeginActor() { return m_actors.begin(); }
	std::list<Actor*>::iterator EndActor() { return m_actors.end(); }

	CameraIter AddCamera(CameraComponent* camera);
	void RemoveCamera(CameraIter iter);

	inline CameraComponent* mainCamera() { return m_mainCamera; }
	void mainCamera(CameraComponent* camera);

	inline PhysicsScene* physicsScene() { return m_physicsScene; }

	void AttachPlayerCamera();

	void Stat();

private:
	void f_Update();
	void Destroy() override;

	void f_FixedUpdate();

private:
	void m_InitMono();

	void m_UpdateActors(std::list<Actor*>* list);
	void m_DestroyActors(std::list<Actor*>* list);

	void m_FixedUpdate(std::list<Actor*>* list);

	void m_MoveToStatic(Actor* actor);

	GameObjectInfo m_CreateActorFromCs(CsRef csRef, CppRef parentRef);

	std::list<Actor*>::iterator m_EraseActor(std::list<Actor*>::iterator it, std::list<Actor*>* list);

};

PUSH_ASSET(Scene);
PROP_GETSET_STR(Scene, name);