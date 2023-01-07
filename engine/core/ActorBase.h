#pragma once

#include <string>

#include "Transform.h"
#include "ActorConcepts.h"

class Component;
class Game;
class Scene;

class ActorBase : public CsLink, public ActorTransform {
	friend class Scene;
	friend class Actor;

private:
	//Transform* transform;

private:

	Actor* friend_gameObject = nullptr;
	Component* friend_component = nullptr;
	int friend_timeToDestroy = 1;
	//bool friend_isStarted = false;

	bool m_onPreDestroy = false;
	

public:
	ActorBase() {}

	Game* game();
	Actor* actor();
	Scene* scene();

	bool HasParent();
	Actor* parent();
	void parent(ActorBase* parent);

	bool IsDestroyed();

	Actor* CreateActor(std::string name = "");
	void Destroy();


	template<IsCppAddableComponent TComponent, typename = std::enable_if_t<std::is_base_of_v<Component, TComponent>>>
	TComponent* AddComponent(bool isRuntimeOnly = false);

	template<typename TComponent, typename = std::enable_if_t<std::is_base_of_v<Component, TComponent>>>
	TComponent* GetComponent();

	template<typename TComponent, typename = std::enable_if_t<std::is_base_of_v<Component, TComponent>>>
	TComponent* GetComponentInChild();

	virtual void RecieveGameMessage(const std::string& msg) {}

private:
	bool friend_CanDestroy() { return !IsDestroyed() && !m_onPreDestroy; }
	void friend_StartDestroy() { m_onPreDestroy = true; }
};
