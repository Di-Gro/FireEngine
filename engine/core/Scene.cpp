#include "Scene.h"

#include "Game.h"
#include "Actor.h"

unsigned int Scene::m_objectCount = 0;

mono::mono_method_invoker<CppRef()> Scene::mono_create;

bool Scene::mono_inited = false;


void Scene::Init(Game* game) {
	m_game = game;

	m_InitMono();
}

void Scene::m_InitMono() {
	if (mono_inited)
		return;

	auto type_Actor = m_game->mono()->GetType("Engine", "Actor");
	mono_create = mono::make_method_invoker<CppRef()>(type_Actor, "cpp_Create");
}

void Scene::f_Update() {
	m_game->PushScene(this);

	auto it = m_actors.begin();
	while (it != m_actors.end()) {

		auto actor = (*it);
		if (actor->IsDestroyed()) {
			actor->friend_timeToDestroy--;
			if (actor->friend_timeToDestroy <= 0)
				it = m_EraseActor(it);
			continue;
		}
		actor->f_Update();
		it++;
	}
	m_game->PopScene();
}

void Scene::f_Destroy() {
	auto it = m_actors.begin();
	while (it != m_actors.end()) {
		DestroyActor(*it);
		it = m_EraseActor(it);
	}
}

void Scene::f_RemoveActor(Actor* actor) {
	//TODO: ”ндалить актор из списка без разрушени€
}

void Scene::f_AddActor(Actor* actor) {
	//TODO: ƒобавить уже существующий актор
}

Actor* Scene::CreateActor(Actor* parent, std::string name) {
	if (m_game->scene() != this)
		throw std::exception("You are trying to create an actor while updating another scene without calling PushScene() / PopScene() pair.");

	auto cppRef = mono_create();

	Actor* actor = CppRefs::ThrowPointer<Actor>(cppRef);
	if (name != "")
		actor->SetName(name);

	if (parent != nullptr) {
		actor->parent(parent);
		actor->localScale({ 1, 1, 1 });
	}
	return actor;
}


GameObjectInfo Scene::m_CreateActorFromCs(CsRef csRef, CppRef parentRef) {
	auto classInfoRef = CppRefs::Create(ClassInfo::Get<Actor>());

	Actor* actor = new Actor();
	m_actors.insert(m_actors.end(), actor);

	actor->f_actorID = ++m_objectCount;
	actor->f_ref = CppRefs::Create(actor);
	actor->f_cppRef = actor->f_ref.cppRef();
	actor->f_csRef = csRef;

	actor->f_Init(m_game, this);

	if (parentRef.value != 0) {
		auto parent = CppRefs::ThrowPointer<Actor>(parentRef);
		actor->parent(parent);
		actor->localScale({ 1, 1, 1 });
	}
	GameObjectInfo objectInfo;
	objectInfo.classRef = RefCpp(classInfoRef.cppRef());
	objectInfo.objectRef = actor->cppRef();

	return objectInfo;
}

void Scene::DestroyActor(Actor* actor) {
	if (actor->ActorBase::friend_CanDestroy()) {
		actor->ActorBase::friend_StartDestroy();

		actor->f_Destroy();

		if (CppRefs::IsValid(actor->f_ref)) {
			CppRefs::Remove(actor->f_ref);
		}
	}
}

std::list<Actor*>::iterator Scene::m_EraseActor(std::list<Actor*>::iterator it) {
	auto* actor = *it;
	auto next = m_actors.erase(it);
	delete actor;
	return next;
}


std::list<Actor*>::iterator Scene::GetNextRootActors(const std::list<Actor*>::iterator& iter)
{
	for (auto it = iter; it != m_actors.end(); ++it)
	{
		if (!(*it)->HasParent())
			return it;
	}
	return m_actors.end();
}

int Scene::GetRootActorsCount() {
	int count = 0;
	for (auto actor : m_actors) {
		if (!actor->HasParent())
			count++;
	}
	return count;
}

void Scene::WriteRootActorsRefs(CsRef* refs) {
	CsRef* ptr = refs;
	for (auto actor : m_actors) {
		if (!actor->HasParent()) {
			*ptr = actor->csRef();
			ptr++;
		}
	}
}

void Scene::MoveActor(Actor* from, Actor* to, bool isPastBefore)
{
	bool isFrom = false;
	bool isTo = false;
	
	std::list<Actor*>::iterator newIt = m_actors.end();

	// Throw Exception
	auto it = m_actors.begin();
	while (it != m_actors.end())
	{
		if (*it == from)
		{
			auto iter = m_actors.erase(it);
			it = iter;
			isFrom = true;
			continue;
		}
		
		if (*it == to)
		{
			newIt = it;
			isTo = true;
		}
	
		if (isFrom && isTo)
			break;

		it++;
	}
	
	if (newIt == m_actors.end())
		return;

	if(isPastBefore)
		m_actors.insert(newIt, from);
	else
		m_actors.insert(++newIt, from);
}


DEF_FUNC(Game, CreateGameObjectFromCS, GameObjectInfo)(CppRef sceneRef, CsRef csRef, CppRef parentRef) {
	return CppRefs::ThrowPointer<Scene>(sceneRef)->m_CreateActorFromCs(csRef, parentRef);
}

DEF_FUNC(Game, GetRootActorsCount, int)(CppRef sceneRef) {
	return CppRefs::ThrowPointer<Scene>(sceneRef)->GetRootActorsCount();
}

DEF_FUNC(Game, WriteRootActorsRefs, void)(CppRef sceneRef, CsRef* refs) {
	CppRefs::ThrowPointer<Scene>(sceneRef)->WriteRootActorsRefs(refs);
}