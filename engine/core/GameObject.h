#pragma once

#include <stdio.h>
#include <string>
#include <iostream>
#include <list>
#include <vector>

#include "GameObjectBase.h"

class Render;

class GameObject : public GameObjectBase {
	friend class Game;
	friend class GameObjectBase;
	friend class Render;

public:
	std::string name = "";
	Transform transform;

private:
	Game* friend_game = nullptr;
	int friend_objectID;

	GameObject* friend_parent = nullptr;

	std::list<Component*> m_components;
	std::vector<GameObject*> m_childs;


public:
	int Id() { return friend_objectID; }

	//bool HasParent() { return friend_parent != nullptr; }
	//GameObject* GetParent() { return friend_parent; }
	//void SetParent(GameObject* gameObject);

	template<typename TComponent, typename = std::enable_if_t<std::is_base_of_v<Component, TComponent>>>
	TComponent* AddComponent();

	template<typename TComponent, typename = std::enable_if_t<std::is_base_of_v<Component, TComponent>>>
	TComponent* GetComponent();

	template<typename TComponent, typename = std::enable_if_t<std::is_base_of_v<Component, TComponent>>>
	TComponent* GetComponentInChild();

	void RecieveGameMessage(const std::string& msg) override;

private:

	GameObject();
	GameObject(const GameObject&) = delete;
	GameObject(GameObject&&) = delete;

	void f_DestroyComponent(Component* component);

	void f_Init(Game* game, std::string name = "");
	void f_Update();
	void f_Draw();
	void f_DrawUI();
	void f_Destroy();

	void f_SetParent(GameObject* gameObject);

	void m_InitComponent(Component* component);
	std::list<Component*>::iterator m_EraseComponent(std::list<Component*>::iterator it);

	void m_DeleteFromParent();
};

#include "GameObject.inl"
#include "GameObjectBase.inl"
#include "Component.inl"