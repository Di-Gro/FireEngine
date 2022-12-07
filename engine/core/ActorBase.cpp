#include "ActorBase.h"

#include "Game.h"


Game* ActorBase::game() {
	if (!IsDestroyed())
		return friend_gameObject->f_game;
	return nullptr;
}

Actor* ActorBase::actor() {
	return friend_gameObject;
}

bool ActorBase::IsDestroyed() {
	return friend_gameObject == nullptr;
}

Actor* ActorBase::CreateActor(std::string name) {
	if (!IsDestroyed())
		return game()->CreateActor(name);
	return nullptr;
}

void ActorBase::Destroy() {
	if (!IsDestroyed()) {
		if (friend_component != nullptr)
			actor()->f_DestroyComponent(friend_component);
		else
			game()->DestroyActor(friend_gameObject);
	}
}

bool ActorBase::HasParent() { 
	return friend_gameObject->f_parent != nullptr; 
}

Actor* ActorBase::parent() { 
	return friend_gameObject->f_parent; 
}

void ActorBase::parent(ActorBase* parent) {
	if (parent == nullptr)
		friend_gameObject->f_SetParent(nullptr);
	else
		friend_gameObject->f_SetParent(parent->friend_gameObject);
}