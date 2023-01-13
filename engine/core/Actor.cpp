#include "Actor.h"

#include "SimpleMath.h"

#include "Game.h"
#include "Math.h"
#include "CameraComponent.h"
#include "Render.h"

/// 
/// � Update ���������� ����� ��������� ������� Actor-�. 
/// ����� ������� OnDraw ����������, ����� ���������� ���������� 
/// ������� ������������� ���� ������������ �������� ������ ����.

DEF_OBJECT(Actor, 0) { }

bool Actor::mono_inited;

mono::mono_method_invoker<CsRef(CsRef, size_t, size_t, CppObjectInfo)> Actor::mono_AddComponent;
mono::mono_method_invoker<CppRef(CsRef, size_t, size_t)> Actor::mono_AddCsComponent;


Actor::Actor() {  }

Actor::~Actor() { }

void Actor::m_CreateTransform() {
	transform = &m_transform;
	transform->friend_gameObject = this;
}

void Actor::MoveChild(Actor* from, Actor* to, bool isPastBefore) {
	bool isFrom = false;
	bool isTo = false;

	int newIndex = 0;

	for (int i = 0; i < m_childs.size(); ++i) {
		if (m_childs[i] == from) {
			m_childs.erase(m_childs.begin() + i);
			isFrom = true;
			--i;
			continue;
		} 
		if (m_childs[i] == to) {
			newIndex = i;
			isTo = true;
		} 
		if (isFrom && isTo)
			break;
	}

	if (isPastBefore)
		m_childs.insert(m_childs.begin() + newIndex, from);
	else
		m_childs.insert(m_childs.begin() + (newIndex + 1), from);
}

void Actor::f_Init(Game* game, Scene* scene) {
	pointerForDestroy(this);

	f_game = game;
	f_scene = scene;
	
	m_InitMono();
	m_CreateTransform();
};

void Actor::m_InitMono() {
	if (mono_inited)
		return;

	auto type = game()->mono()->GetType("Engine", "Actor");
	mono_AddComponent = mono::make_method_invoker<CsRef(CsRef, size_t, size_t, CppObjectInfo)>(type, "cpp_AddComponent");
	mono_AddCsComponent = mono::make_method_invoker<CppRef(CsRef, size_t, size_t)>(type, "cpp_AddCsComponent");

	mono_inited = true;
}

void Actor::f_Update() {

	auto it = m_components.begin();
	while (it != m_components.end()) {

		auto component = (*it);
		if (component->IsDestroyed()) {
			component->timeToDestroy--;
			if (component->timeToDestroy <= 0)
				it = m_EraseComponent(it);
			continue;
		}

		if (m_NeedRunComponent(component)) {
			if (!component->f_isStarted)
				m_RunOrCrash(component, &Actor::m_OnStartComponent);
			else
				m_RunOrCrash(component, &Actor::m_OnUpdateComponent);
		}
		it++;
	}
};

void Actor::f_FixedUpdate() {
	for (auto it = m_components.begin(); it != m_components.end(); it++) {
		auto component = (*it);

		if (!component->IsDestroyed() && m_NeedRunComponent(component)) {
			if (!component->f_isStarted)
				m_RunOrCrash(component, &Actor::m_OnStartComponent);
			else
				m_RunOrCrash(component, &Actor::m_OnFixedUpdateComponent);
		}
	}
}

void Actor::f_Destroy() {
	for (int i = m_childs.size() - 1; i >= 0; --i)
		m_childs[i]->Destroy();

	assert(m_childs.size() == 0);

	auto it = m_components.begin();
	while (it != m_components.end()) {
		f_DestroyComponent(*it);
		it = m_EraseComponent(it);
	}
	m_DeleteFromParent();

	f_game = nullptr;
}

void Actor::f_Draw() {
	for (auto component : m_components) {
		if (!component->IsDestroyed()) {
			component->OnDraw();
		}
	}
};


std::list<Component*>::iterator Actor::m_EraseComponent(std::list<Component*>::iterator it) {
	auto* component = *it;
	auto next = m_components.erase(it);
	delete component;
	return next;
}

void Actor::m_BindComponent(Component* component) {
	auto it = m_components.insert(m_components.end(), component);

	component->ActorBase::pointerForDestroy(this);
	component->f_selfActor = this;
	component->ActorBase::friend_component = component;
	component->ActorBase::transform = transform;
}

void Actor::m_InitComponent(Component* component) {
	if (m_NeedRunComponent(component))
		m_RunOrCrash(component, &Actor::m_OnInitComponent);
}

void Actor::f_DestroyComponent(Component* component) {
	if (component->ActorBase::BeginDestroy()) {
		if (m_NeedRunComponent(component))
			m_RunOrCrash(component, &Actor::m_OnDestroyComponent);

		if (CppRefs::IsValid(component->f_ref)) 
			CppRefs::Remove(component->f_ref);
		
		if (component->csRef().value > 0) 
			game()->callbacks().removeCsRef(component->csRef());
		
		component->ActorBase::friend_component = nullptr;
		component->ActorBase::EndDestroy();
	}
}

void Actor::f_SetParent(Actor* actor) {
	if (actor != nullptr && this->f_scene != actor->f_scene)
		throw std::exception("actor.scene != parent.scene");

	if (f_parent == actor)
		return;

	transform->friend_ChangeParent(actor);
	m_DeleteFromParent();
	f_parent = actor;
	
	if (f_parent != nullptr)
		f_parent->m_childs.push_back(this);
}

void Actor::m_DeleteFromParent() {
	if (f_parent != nullptr) {
		for (int i = f_parent->m_childs.size() - 1; i >= 0; --i) {
			if (f_parent->m_childs[i] == this) {
				f_parent->m_childs.erase(f_parent->m_childs.begin() + i);
				break;
			}
		}
		f_parent = nullptr;
	}
}

int Actor::GetComponentsCount() {
	int count = 0;

	for (auto component : m_components) {
		if (!component->IsDestroyed()) {
			count++;
		}
	}
	return count;
}

void Actor::WriteComponentsRefs(size_t* csRefsList) {
	size_t* ptr = csRefsList;

	for (auto component : m_components) {
		if (!component->IsDestroyed()) {
			*ptr = component->csRef();
			ptr++;
		}
	}
}

Actor* Actor::GetChild(int index) {
	if (index >= 0 && index < m_childs.size())
		return m_childs[index];
	return nullptr;
}


static inline Vector3 deg(Vector3 vec) {
	return Vector3(deg(vec.x), deg(vec.y), deg(vec.z));
}

static std::string ToString(Transform& transform) {
	std::string str;

	str += "local position: " + ToString(transform.localPosition()) + "\n";
	str += "local rotation: " + ToString(deg(transform.localRotation())) + "\n";

	str += "world position: " + ToString(transform.worldPosition()) + "\n";
	str += "world rotation: " + ToString(deg(transform.worldRotation())) + "\n";

	return str;
}

void Actor::RecieveGameMessage(const std::string& msg) { }

bool Actor::m_NeedRunComponent(Component* component) {
	if (component->f_isCrashed)
		return false;

	if (scene()->isEditor())
		return component->NeedRunInEditor();
	return component->NeedRunInPlayer();
}

void Actor::m_RunOrCrash(Component* component, void (Actor::* func)(Component*)) {
	try {
		(this->*func)(component);
	}
	catch (std::exception ex) {
		component->f_isCrashed = true;

		auto className = component->GetMeta().name;
		auto actorName = component->actor()->name();
		auto actorId = std::to_string(component->actor()->Id());

		std::cout << "ComponentCrash: Component was disabled.\n";
		std::cout << "Component class: " << className << ", ";
		std::cout <<  "Actor {name: " << actorName << ", ID : " << actorId << "}\n";
		std::cout << "Exception: \n";
		std::cout << ex.what() << "\n";
	}
}

void Actor::m_OnInitComponent(Component* component) {
	if (!m_NeedRunComponent(component))
		return;

	component->OnInit();
	component->f_isInited = true;
	if (component->csRef().value > 0 && component->f_callbacks.onInit != nullptr) {
		auto method = component->f_callbacks.onInit;
		auto runOrCrash = game()->callbacks().runOrCrush;

		component->f_isCrashed |= runOrCrash(component->csRef(), method);
	}
}

void Actor::m_OnStartComponent(Component* component) {
	if(!component->f_isInited)
		throw std::exception("Component is not inited.");

	component->OnStart();
	component->f_isStarted = true;
	if (component->csRef().value > 0 && component->f_callbacks.onStart != nullptr) {
		auto method = component->f_callbacks.onStart;
		auto runOrCrash = game()->callbacks().runOrCrush;

		component->f_isCrashed |= runOrCrash(component->csRef(), method);
	}
}

void Actor::m_OnUpdateComponent(Component* component) {
	if (!component->f_isStarted)
		throw std::exception("Component is not started.");

	component->OnUpdate();
	if (component->csRef().value > 0 && component->f_callbacks.onUpdate != nullptr) {
		auto method = component->f_callbacks.onUpdate;
		auto runOrCrash = game()->callbacks().runOrCrush;

		component->f_isCrashed |= runOrCrash(component->csRef(), method);
	}
}

inline void Actor::m_OnFixedUpdateComponent(Component* component) {
	if (!component->f_isStarted)
		throw std::exception("Component is not started.");

	auto id = component->actor()->Id();
	component->OnFixedUpdate();
	if (component->csRef().value > 0 && component->f_callbacks.onFixedUpdate != nullptr) {
		auto method = component->f_callbacks.onFixedUpdate;
		auto runOrCrash = game()->callbacks().runOrCrush;

		component->f_isCrashed |= runOrCrash(component->csRef(), method);
	}
}

void Actor::m_OnDestroyComponent(Component* component) {
	if (!component->f_isInited)
		return;

	component->OnDestroy();
	if (component->csRef().value > 0 && component->f_callbacks.onDestroy != nullptr) {
		auto method = component->f_callbacks.onDestroy;
		auto runOrCrash = game()->callbacks().runOrCrush;

		component->f_isCrashed |= runOrCrash(component->csRef(), method);
	}
}

void Actor::m_SetComponentCallbacks(Component* component, ComponentCallbacks callbacks) {
	component->f_callbacks = callbacks;
}


DEF_FUNC(Actor, gameObject_get, CsRef)(CppRef objBaseRef) {
	return CppRefs::ThrowPointer<ActorBase>(objBaseRef)->actor()->csRef();
}

DEF_FUNC(Actor, parent_get, CsRef)(CppRef objRef) {
	auto parent = CppRefs::ThrowPointer<Actor>(objRef)->parent();
	return parent != nullptr ? parent->csRef() : CsRef();
}

DEF_FUNC(Actor, parent_set, void)(CppRef objRef, CppRef newObjRef) {
	//std::cout << "+: GameObject_parent_set(): " << objRef << " -> " << newObjRef << std::endl;

	auto object = CppRefs::ThrowPointer<Actor>(objRef);
	auto parent = CppRefs::ThrowPointer<Actor>(newObjRef);
	object->parent(parent);
}

void Actor_BindComponent(CppRef objRef, CppRef compRef) {
	auto* actor = CppRefs::ThrowPointer<Actor>(objRef);
	auto* component = CppRefs::ThrowPointer<Component>(compRef);
	actor->m_BindComponent(component);
}

void Actor_InitComponent(CppRef objRef, CppRef compRef) {
	auto* actor = CppRefs::ThrowPointer<Actor>(objRef);
	auto* component = CppRefs::ThrowPointer<Component>(compRef);
	actor->m_InitComponent(component);
}

DEF_FUNC(Actor, DestroyComponent, void)(CppRef compRef) {
	CppRefs::ThrowPointer<Component>(compRef)->Destroy();
}

DEF_FUNC(Actor, Destroy, void)(CppRef objRef) {
	CppRefs::ThrowPointer<Actor>(objRef)->Destroy();
}

DEF_FUNC(Actor, GetComponentsCount, int)(CppRef objRef) {
	return CppRefs::ThrowPointer<Actor>(objRef)->GetComponentsCount();
}

DEF_FUNC(Actor, WriteComponentsRefs, void)(CppRef objRef, size_t listPtr) {
	CppRefs::ThrowPointer<Actor>(objRef)->WriteComponentsRefs((size_t*)listPtr);
}

DEF_FUNC(Actor, GetChildrenCount, int)(CppRef objRef) {
	return CppRefs::ThrowPointer<Actor>(objRef)->GetChildrenCount();
}

DEF_FUNC(Actor, GetChild, CsRef)(CppRef objRef, int index) {
	auto child = CppRefs::ThrowPointer<Actor>(objRef)->GetChild(index);
	return child != nullptr ? child->csRef() : CsRef();
}

DEF_PROP_GETSET_STR(Actor, name);

DEF_PROP_GETSET(Actor, Vector3, localPosition)
DEF_PROP_GETSET(Actor, Vector3, localRotation)
DEF_PROP_GETSET(Actor, Quaternion, localRotationQ)
DEF_PROP_GETSET(Actor, Vector3, localScale)

DEF_PROP_GETSET(Actor, Vector3, worldPosition)
DEF_PROP_GETSET(Actor, Quaternion, worldRotationQ)
DEF_PROP_GETSET(Actor, Vector3, worldScale)

DEF_PROP_GETSET_F(Component, bool, runtimeOnly, runtimeOnly);
DEF_PROP_GETSET_F(Component, bool, f_isCrashed, f_isCrashed);

DEF_PROP_GET(Actor, Vector3, localForward)
DEF_PROP_GET(Actor, Vector3, localUp)
DEF_PROP_GET(Actor, Vector3, localRight)

DEF_PROP_GET(Actor, Vector3, forward)
DEF_PROP_GET(Actor, Vector3, up)
DEF_PROP_GET(Actor, Vector3, right)

DEF_FUNC(Actor, SetComponentCallbacks, void)(CppRef componentRef, const ComponentCallbacks& callbacks) {
	auto component = CppRefs::ThrowPointer<Component>(componentRef);
	Actor::m_SetComponentCallbacks(component, callbacks);
}

DEF_FUNC(Actor, scene_get, CppRef)(CppRef objRef) {
	auto actor = CppRefs::ThrowPointer<Actor>(objRef);
	return CppRefs::GetRef(actor->scene());
}