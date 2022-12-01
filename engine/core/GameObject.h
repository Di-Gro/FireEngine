#pragma once

#include <stdio.h>
#include <string>
#include <iostream>
#include <list>
#include <vector>

#include "CSLinked.h"
#include "CSBridge.h"

#include "GameObjectConcepts.h"
#include "GameObjectBase.h"


extern "C" __declspec(dllexport) void GameObject_InitComponent(CppRef objRef, CppRef compRef);


class Render;

class GameObject : public GameObjectBase {
	OBJECT;

	friend class Game;
	friend class GameObjectBase;
	friend class Render;

	friend void GameObject_InitComponent(CppRef objRef, CppRef compRef);

public:
	std::string name = "";

private:
	Game* f_game = nullptr;
	int f_objectID;

	GameObject* f_parent = nullptr;

	std::list<Component*> m_components;
	std::vector<GameObject*> m_childs;

	static bool mono_inited;
	static mono::mono_method_invoker<CsRef(CsRef, size_t, size_t, CppObjectInfo)> mono_AddComponent;
	static mono::mono_method_invoker<CppRef(CsRef, size_t, size_t)> mono_AddCsComponent;
	static mono::mono_method_invoker<CsRef(CppRef, CppRef)> mono_CreateTransform;
	static mono::mono_method_invoker<void(CsRef)> mono_RemoveTransform;

	static mono::mono_method_invoker<void(CsRef)> mono_OnInit;
	static mono::mono_method_invoker<void(CsRef)> mono_OnStart;
	static mono::mono_method_invoker<void(CsRef)> mono_OnUpdate;
	static mono::mono_method_invoker<void(CsRef)> mono_OnDestroy;


public:

	int Id() { return f_objectID; }

	template<typename TComponent, typename = std::enable_if_t<std::is_base_of_v<Component, TComponent>>>
	TComponent* CreateComponent(CsRef csRef = CsRef::Void);

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

	GameObject* GetChild(int index);

	void WriteComponentsRefs(size_t *csRefsList);

	void RecieveGameMessage(const std::string& msg) override;

	~GameObject();

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

	std::list<Component*>::iterator m_EraseComponent(std::list<Component*>::iterator it);

	void m_DeleteFromParent();

	void m_InitMono();
	void m_CreateTransform();
	void m_RemoveTransform();

	void m_InitComponent(Component* component);

	inline void m_OnInitComponent(Component* component);
	inline void m_OnStartComponent(Component* component);
	inline void m_OnUpdateComponent(Component* component);
	inline void m_OnDestroyComponent(Component* component);

};

FUNC(GameObject, gameObject_get, CsRef)(CppRef objBaseRef);

FUNC(GameObject, parent_get, CsRef)(CppRef objRef);
FUNC(GameObject, parent_set, void)(CppRef objRef, CppRef newObjRef);

FUNC(GameObject, DestroyComponent, void)(CppRef compRef);
FUNC(GameObject, Destroy, void)(CppRef objRef);

FUNC(GameObject, GetComponentsCount, int)(CppRef objRef);
FUNC(GameObject, WriteComponentsRefs, void)(CppRef objRef, size_t listPtr);

FUNC(GameObject, GetChildrenCount, int)(CppRef objRef);
FUNC(GameObject, GetChild, CsRef)(CppRef objRef, int index);

#include "GameObject.inl"
#include "GameObjectBase.inl"
#include "Component.inl"