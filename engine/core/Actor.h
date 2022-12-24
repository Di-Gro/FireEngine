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

FUNC(Actor, InitComponent, void)(CppRef objRef, CppRef compRef);
FUNC(Actor, SetComponentCallbacks, void)(CppRef componentRef, const ComponentCallbacks& callbacks);

class Render;

class Actor : public ActorBase {
	OBJECT;

	friend class Game;
	friend class ActorBase;
	friend class Render;

	FRIEND_FUNC(Actor, InitComponent, void)(CppRef objRef, CppRef compRef);
	FRIEND_FUNC(Actor, SetComponentCallbacks, void)(CppRef componentRef, const ComponentCallbacks& callbacks);

public:
	std::string name = "";

private:
	Game* f_game = nullptr;
	unsigned int f_actorID;

	Actor* f_parent = nullptr;

	Transform m_transform;
	std::list<Component*> m_components;
	std::vector<Actor*> m_childs;

	static bool mono_inited;
	static mono::mono_method_invoker<CsRef(CsRef, size_t, size_t, CppObjectInfo)> mono_AddComponent;
	static mono::mono_method_invoker<CppRef(CsRef, size_t, size_t)> mono_AddCsComponent;
	static mono::mono_method_invoker<void(CsRef, size_t, size_t)> mono_SetName;


public:

	unsigned int Id() { return f_actorID; }

	void SetName(const std::string& value);
	const std::string& GetName() { return name; }

	template<typename TComponent, typename = std::enable_if_t<std::is_base_of_v<Component, TComponent>>>
	TComponent* inner_CreateComponent(CsRef csRef = CsRef::Void);

	template<IsCppAddableComponent TComponent, typename = std::enable_if_t<std::is_base_of_v<Component, TComponent>>>
	TComponent* AddComponent();

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

	~Actor();

private:

	Actor();
	Actor(const Actor&) = delete;
	Actor(Actor&&) = delete;

	void f_DestroyComponent(Component* component);

	void f_Init(Game* game);
	void f_Update();
	void f_Draw();
	void f_DrawUI();
	void f_Destroy();

	void f_SetParent(Actor* actor);

	std::list<Component*>::iterator m_EraseComponent(std::list<Component*>::iterator it);

	void m_DeleteFromParent();

	void m_InitMono();
	void m_CreateTransform();
	//void m_RemoveTransform();

	void m_InitComponent(Component* component);
	void m_SetComponentCallbacks(Component* component, ComponentCallbacks callbacks);

	inline void m_OnInitComponent(Component* component);
	inline void m_OnStartComponent(Component* component);
	inline void m_OnUpdateComponent(Component* component);
	inline void m_OnDestroyComponent(Component* component);

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


PROP_GETSET(Actor, Vector3, localPosition)
PROP_GETSET(Actor, Vector3, localRotation)
PROP_GETSET(Actor, Quaternion, localRotationQ)
PROP_GETSET(Actor, Vector3, localScale)

PROP_GETSET(Actor, Vector3, worldPosition)
PROP_GETSET(Actor, Quaternion, worldRotationQ)
PROP_GETSET(Actor, Vector3, worldScale)

PROP_GET(Actor, Vector3, localForward)
PROP_GET(Actor, Vector3, localUp)
PROP_GET(Actor, Vector3, localRight)

PROP_GET(Actor, Vector3, forward)
PROP_GET(Actor, Vector3, up)
PROP_GET(Actor, Vector3, right)


#pragma warning( pop ) 

#include "Actor.inl"
#include "ActorBase.inl"
#include "Component.inl"