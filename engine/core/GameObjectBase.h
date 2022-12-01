#pragma once

#include <string>

#include "Transform.h"
#include "GameObjectConcepts.h"


class GameObjectBase : public CsLink {
	friend class Game;
	friend class GameObject;

public:
	Transform* transform;

private:

	//Ref2<GameObjectBase> f_ref;
	//CppRef f_cppRef;
	//CsRef f_csRef;

	GameObject* friend_gameObject = nullptr; // <- ÑC
	Component* friend_component = nullptr;
	int friend_timeToDestroy = 1;
	bool friend_isStarted = false;

	bool m_onPreDestroy = false;
	

public:
	GameObjectBase() {}

	Game* game();
	GameObject* gameObject();

	//CppRef cppRef() { return f_cppRef; }
	//CsRef csRef() { return f_csRef; }

	bool IsDestroyed();

	bool HasParent();
	GameObject* GetParent();
	void SetParent(GameObjectBase* parent);


	GameObject* CreateGameObject(std::string name = "");
	void Destroy();


	template<IsCppAddableComponent TComponent, typename = std::enable_if_t<std::is_base_of_v<Component, TComponent>>>
	TComponent* AddComponent();

	template<typename TComponent, typename = std::enable_if_t<std::is_base_of_v<Component, TComponent>>>
	TComponent* GetComponent();

	template<typename TComponent, typename = std::enable_if_t<std::is_base_of_v<Component, TComponent>>>
	TComponent* GetComponentInChild();

	virtual void RecieveGameMessage(const std::string& msg) {}

private:
	bool friend_CanDestroy() { return !IsDestroyed() && !m_onPreDestroy; }
	void friend_StartDestroy() { m_onPreDestroy = true; }
};
