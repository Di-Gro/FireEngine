#pragma once

#include <stdio.h>
#include <string>
#include <iostream>
#include <list>
#include <vector>

#include "CSLinked.h"
#include "CSBridge.h"

#include "ActorConcepts.h"
#include "ActorBase.h"
#include "RunMode.h"

#include "Contact.h"

FUNC(Actor, BindComponent, void)(CppRef objRef, CppRef compRef);
FUNC(Actor, InitComponent, void)(CppRef objRef, CppRef compRef);
FUNC(Actor, SetComponentCallbacks, void)(CppRef componentRef, const ComponentCallbacks& callbacks);

class Render;
class Game;
class Scene;
class FireContactListener;

enum class BodyTag {
	Null = 0,
	Trigger = 1,
};

class Actor : public ActorBase {
	OBJECT;

	friend class Scene;
	friend class ActorBase;
	friend class Render;
	friend class FireContactListener;

	FRIEND_FUNC(Actor, BindComponent, void)(CppRef objRef, CppRef compRef);
	FRIEND_FUNC(Actor, InitComponent, void)(CppRef objRef, CppRef compRef);
	FRIEND_FUNC(Actor, SetComponentCallbacks, void)(CppRef componentRef, const ComponentCallbacks& callbacks);

public:
	size_t flags = 0;
	BodyTag bodyTag = BodyTag::Null;

	bool isSelectable = true;
	//int prefabIdHash = 0;

private:
	std::string m_name = "";
	std::string m_prefabId = "";

	bool m_isActiveSelf = true;

	Game* f_game = nullptr;
	Scene* f_scene = nullptr;
	unsigned int f_actorID;

	Actor* f_parent = nullptr;

	Transform m_transform;
	std::list<Component*> m_components;
	std::vector<Actor*> m_childs;

	static bool mono_inited;
	static mono::mono_method_invoker<CsRef(CsRef, size_t, size_t, CppObjectInfo)> mono_AddComponent;
	static mono::mono_method_invoker<CppRef(CsRef, size_t, size_t)> mono_AddCsComponent;
	//static mono::mono_method_invoker<void(CsRef, size_t, size_t)> mono_SetName;

public:
	unsigned int Id() const { return f_actorID; }

	bool isActive();
	bool activeSelf() { return m_isActiveSelf; }
	void activeSelf(bool value);

	const std::string& name() { return m_name; }
	void name(const std::string& value) { m_name = value; }

	const std::string& prefabId() { return m_prefabId; }
	void prefabId(const std::string& value) { m_prefabId = value; }

	//const std::string& postfix() { return m_postfix; }
	//void postfix(const std::string& value) { m_postfix = value; }

	//std::string fullname() { return m_name + " " + m_postfix; }

	void MoveChild(Actor* from, Actor* v_to, bool isPastBefore);

	template<typename TComponent, typename = std::enable_if_t<std::is_base_of_v<Component, TComponent>>>
	static TComponent* inner_CreateComponent(CsRef csRef = CsRef::Void);

	template<IsCppAddableComponent TComponent, typename = std::enable_if_t<std::is_base_of_v<Component, TComponent>>>
	TComponent* AddComponent(bool isRuntimeOnly = false);

	template<typename TComponent, typename = std::enable_if_t<std::is_base_of_v<Component, TComponent>>>
	TComponent* AddComponent(const std::string& className);

	template<typename TComponent, typename = std::enable_if_t<std::is_base_of_v<Component, TComponent>>>
	TComponent* GetComponent();

	template<typename TComponent, typename = std::enable_if_t<std::is_base_of_v<Component, TComponent>>>
	TComponent* GetComponentInChild();

	int GetChildrenCount() { return m_childs.size(); }
	int GetComponentsCount();

	Actor* GetChild(int index);

	void WriteComponentsRefs(size_t *csRefsList);

	const std::list<Component*>* GetComponentList() {
		return &m_components;
	};

	void RecieveGameMessage(const std::string& msg) override;

	void Clear();

	~Actor();

private:

	Actor();
	Actor(const Actor&) = delete;
	Actor(Actor&&) = delete;

	void f_DestroyComponent(Component* component);

	void f_Init(Game* game, Scene* scene);
	void f_Update();
	void f_FixedUpdate();
	void f_Draw();
	//void f_DrawUI();
	void f_Destroy();

	void f_EnterCollision(Actor* otherActor, const Contact& contact);
	void f_ExitCollision(Actor* otherActor);

	void f_EnterTrigger(Actor* otherActor, const Contact& contact);
	void f_ExitTrigger(Actor* otherActor);
	
	void f_SetParent(Actor* actor);

	std::list<Component*>::iterator m_EraseComponent(std::list<Component*>::iterator it);

	void m_DeleteFromParent();

	inline bool m_NeedRunComponent(Component* component);

	void m_InitMono();
	void m_CreateTransform();
	//void m_RemoveTransform();

	void m_BindComponent(Component* component);
	void m_InitComponent(Component* component);

	static void m_SetComponentCallbacks(Component* component, ComponentCallbacks callbacks);

	inline void m_OnInitComponent(Component* component);
	inline void m_OnInitDisabledComponent(Component* component);

	inline void m_OnStartComponent(Component* component);
	inline void m_OnUpdateComponent(Component* component);
	inline void m_OnFixedUpdateComponent(Component* component);

	inline void m_OnDestroyComponent(Component* component);
	inline void m_OnDestroyDisabledComponent(Component* component);

	inline void m_OnActivateComponent(Component* component);
	inline void m_OnDeactivateComponent(Component* component);

	inline void m_OnCollisionEnterComponent(Component* component, Actor* otherActor, const Contact& contact);
	inline void m_OnCollisionExitComponent(Component* component, Actor* otherActor);

	inline void m_OnTriggerEnterComponent(Component* component, Actor* otherActor, const Contact& contact);
	inline void m_OnTriggerExitComponent(Component* component, Actor* otherActor);

	inline void m_RunOrCrash(Component* component, void (Actor::* func)(Component*));
	inline void m_RunOrCrash(Component* component, void (Actor::* func)(Component*, Actor*, const Contact&), Actor* otherActor, const Contact& contact);
	inline void m_RunOrCrash(Component* component, void (Actor::* func)(Component*, Actor*), Actor* otherActor);

	inline void m_CrashComponent(Component* component, std::exception ex);

	void m_OnActiveChanged(bool value, bool self);

};
#pragma warning( push )
#pragma warning( disable : 4190)


FUNC(Actor, gameObject_get, CsRef)(CppRef objBaseRef);

FUNC(Actor, parent_get, CsRef)(CppRef objRef);
FUNC(Actor, parent_set, void)(CppRef objRef, CppRef newObjRef);

FUNC(Actor, DestroyComponent, void)(CppRef compRef);
FUNC(Actor, Destroy, void)(CppRef objRef);

FUNC(Actor, GetComponentsCount, int)(CppRef objRef);
FUNC(Actor, WriteComponentsRefs, void)(CppRef objRef, size_t listPtr);

FUNC(Actor, GetChildrenCount, int)(CppRef objRef);
FUNC(Actor, GetChild, CsRef)(CppRef objRef, int index);

PROP_GETSET(Actor, size_t, flags)

PROP_GETSET_STR(Actor, name);
PROP_GETSET_STR(Actor, prefabId);

PROP_GET(Actor, bool, isActive);
PROP_GETSET(Actor, bool, activeSelf);

PROP_GETSET(Actor, Vector3, localPosition)
PROP_GETSET(Actor, Vector3, localRotation)
PROP_GETSET(Actor, Quaternion, localRotationQ)
PROP_GETSET(Actor, Vector3, localScale)

PROP_GETSET(Actor, Vector3, worldPosition)
PROP_GETSET(Actor, Quaternion, worldRotationQ)
PROP_GETSET(Actor, Vector3, worldScale)

PROP_GETSET(Component, bool, runtimeOnly);
PROP_GETSET(Component, bool, f_isCrashed);
PROP_GET(Component, bool, IsActivated);

PROP_GET(Actor, Vector3, localForward)
PROP_GET(Actor, Vector3, localUp)
PROP_GET(Actor, Vector3, localRight)

PROP_GET(Actor, Vector3, forward)
PROP_GET(Actor, Vector3, up)
PROP_GET(Actor, Vector3, right)

FUNC(Actor, scene_get, CppRef)(CppRef objRef);


#pragma warning( pop )

#include "Actor.inl"
#include "ActorBase.inl"
#include "Component.inl"
