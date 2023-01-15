#include "ActorBase.h"

#include "Game.h"
#include "Scene.h"
#include "Actor.h"


Scene* ActorBase::scene() {
	if (!IsDestroyed())
		return pointerForDestroy<Actor>()->f_scene;
	return nullptr;
}

Game* ActorBase::game() {
	if (!IsDestroyed())
		return pointerForDestroy<Actor>()->f_game;
	return nullptr;
}

Actor* ActorBase::actor() {
	return pointerForDestroy<Actor>();
}

Actor* ActorBase::CreateActor(std::string name) {
	if (!IsDestroyed())
		return game()->currentScene()->CreateActor(name);
	return nullptr;
}

void ActorBase::Destroy() {
	if (!IsDestroyed()) {
		if (friend_component != nullptr)
			actor()->f_DestroyComponent(friend_component);
		else
			scene()->DestroyActor(pointerForDestroy<Actor>());
	}
}

bool ActorBase::HasParent() { 
	return pointerForDestroy<Actor>()->f_parent != nullptr;
}

Actor* ActorBase::parent() { 
	return pointerForDestroy<Actor>()->f_parent;
}

void ActorBase::parent(ActorBase* parent) {
	if (parent == nullptr)
		pointerForDestroy<Actor>()->f_SetParent(nullptr);
	else
		pointerForDestroy<Actor>()->f_SetParent(parent->pointerForDestroy<Actor>());
}
