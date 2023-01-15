#pragma once

#include <string>

#include "Transform.h"
#include "ActorConcepts.h"
#include "SafeDestroy.h"

class Component;
class Game;
class Scene;
class Actor;

class ActorBase : public CsLink, public ActorTransform, public SafeDestroy {
	friend class Scene;
	friend class Actor;

private:

	Component* friend_component = nullptr;

public:
	ActorBase() {}

	Game* game();
	Actor* actor();
	Scene* scene();

	bool HasParent();
	Actor* parent();
	void parent(ActorBase* parent);

	Actor* CreateActor(std::string name = "");
	void Destroy() override;


	template<IsCppAddableComponent TComponent, typename = std::enable_if_t<std::is_base_of_v<Component, TComponent>>>
	TComponent* AddComponent(bool isRuntimeOnly = false);

	template<typename TComponent, typename = std::enable_if_t<std::is_base_of_v<Component, TComponent>>>
	TComponent* GetComponent();

	template<typename TComponent, typename = std::enable_if_t<std::is_base_of_v<Component, TComponent>>>
	TComponent* GetComponentInChild();

	virtual void RecieveGameMessage(const std::string& msg) {}

};
