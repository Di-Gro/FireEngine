#include "GameObjectBase.h"

#include "Game.h"


Game* GameObjectBase::game() {
	if (!IsDestroyed())
		return friend_gameObject->f_game;
	return nullptr;
}

GameObject* GameObjectBase::gameObject() {
	return friend_gameObject;
}

bool GameObjectBase::IsDestroyed() {
	return friend_gameObject == nullptr;
}

GameObject* GameObjectBase::CreateGameObject(std::string name) {
	if (!IsDestroyed())
		return game()->CreateGameObject(name);
	return nullptr;
}

void GameObjectBase::Destroy() {
	if (!IsDestroyed()) {
		if (friend_component != nullptr)
			gameObject()->f_DestroyComponent(friend_component);
		else
			game()->DestroyGameObject(friend_gameObject);
	}
}

bool GameObjectBase::HasParent() { 
	return friend_gameObject->f_parent != nullptr; 
}

GameObject* GameObjectBase::GetParent() { 
	return friend_gameObject->f_parent; 
}

void GameObjectBase::SetParent(GameObjectBase* parent) {
	if (parent == nullptr)
		friend_gameObject->f_SetParent(nullptr);
	else
		friend_gameObject->f_SetParent(parent->friend_gameObject);
}
