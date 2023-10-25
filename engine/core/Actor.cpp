#include "Actor.h"

#include "SimpleMath.h"

#include "Game.h"
#include "Math.h"
#include "CameraComponent.h"
#include "Render.h"

/// 
/// В Update компоненты МОГУТ обновлять позиции Actor-а. 
/// Перед вызовом OnDraw компонента, нужно рекурсивно рассчитать 
/// матрицы трансформаций всех родительских объектов сверху вниз.

DEF_OBJECT(Actor, 0) { }

bool Actor::mono_inited;

mono::mono_method_invoker<CsRef(CsRef, size_t, size_t, CppObjectInfo)> Actor::mono_AddComponent;
mono::mono_method_invoker<CppRef(CsRef, size_t, size_t)> Actor::mono_AddCsComponent;


Actor::Actor() {  }

Actor::~Actor() { }

bool Actor::isActive() {
	auto actor = this;
	while (actor != nullptr) {

		if (!actor->activeSelf())
			return false;

		actor = actor->parent();
	}
	return true;
}

void Actor::activeSelf(bool value) {
	if (IsDestroyed() || m_isActiveSelf == value)
		return;
	
	m_isActiveSelf = value;

	bool isParentActive = parent() != nullptr ? parent()->isActive() : true;
	if (isParentActive)
		m_OnActiveChanged(m_isActiveSelf, true);
}

void Actor::m_OnActiveChanged(bool value, bool self) {
	if (!self && !m_isActiveSelf)
		return;

	for (auto it = m_components.begin(); it != m_components.end(); it++) {
		auto component = (*it);

		if (!component->IsDestroyed() && m_NeedRunComponent(component) && component->f_isStarted) {
			if (value)
				m_RunOrCrash(component, &Actor::m_OnActivateComponent);
			else
				m_RunOrCrash(component, &Actor::m_OnDeactivateComponent);
		}
	}

	for (auto child : m_childs)
		child->m_OnActiveChanged(value, false);
}

void Actor::m_CreateTransform() {
	transform = &m_transform;
	transform->friend_gameObject = this;
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
	if (!isActive())
		return;

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
			if(!component->f_isInited)
				m_RunOrCrash(component, &Actor::m_OnInitComponent);
			else if (!component->f_isStarted)
				m_RunOrCrash(component, &Actor::m_OnStartComponent);
			else
				m_RunOrCrash(component, &Actor::m_OnUpdateComponent);
		}

		if (IsDestroyed())
			break;

		it++;
	}
};

void Actor::f_FixedUpdate() {
	if (!isActive())
		return;

	for (auto it = m_components.begin(); it != m_components.end(); it++) {
		auto component = (*it);

		if (!component->IsDestroyed() && m_NeedRunComponent(component)) {
			if (component->f_isStarted)
				m_RunOrCrash(component, &Actor::m_OnFixedUpdateComponent);
		}
if (this->IsDestroyed())
break;
	}
}

void Actor::f_EnterCollision(Actor* otherActor, const Contact& contact) {
	assert(isActive());

	for (auto it = m_components.begin(); it != m_components.end(); it++) {
		auto component = (*it);

		if (!component->IsDestroyed() && m_NeedRunComponent(component) && component->f_isStarted) {
			m_RunOrCrash(component, &Actor::m_OnCollisionEnterComponent, otherActor, contact);
		}
	}
}

void Actor::f_ExitCollision(Actor* otherActor) {
	assert(isActive());

	for (auto it = m_components.begin(); it != m_components.end(); it++) {
		auto component = (*it);

		if (!component->IsDestroyed() && m_NeedRunComponent(component) && component->f_isStarted) {
			m_RunOrCrash(component, &Actor::m_OnCollisionExitComponent, otherActor);
		}
	}
}

void Actor::f_EnterTrigger(Actor* otherActor, const Contact& contact) {
	assert(isActive());

	for (auto it = m_components.begin(); it != m_components.end(); it++) {
		auto component = (*it);

		if (!component->IsDestroyed() && m_NeedRunComponent(component) && component->f_isStarted) {
			m_RunOrCrash(component, &Actor::m_OnTriggerEnterComponent, otherActor, contact);
		}
	}
}

void Actor::f_ExitTrigger(Actor* otherActor) {
	assert(isActive());

	for (auto it = m_components.begin(); it != m_components.end(); it++) {
		auto component = (*it);

		if (!component->IsDestroyed() && m_NeedRunComponent(component) && component->f_isStarted) {
			m_RunOrCrash(component, &Actor::m_OnTriggerExitComponent, otherActor);
		}
	}
}

void Actor::f_Destroy() {
	Clear();

	m_DeleteFromParent();

	f_game = nullptr;
}

void Actor::Clear() {
	for (int i = m_childs.size() - 1; i >= 0; --i)
		m_childs[i]->Destroy();

	assert(m_childs.size() == 0);

	auto it = m_components.begin();
	while (it != m_components.end()) {
		f_DestroyComponent(*it);
		it = m_EraseComponent(it);
	}
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
	if (!m_NeedRunComponent(component))
		return;

	if (isActive())
		m_RunOrCrash(component, &Actor::m_OnInitComponent);
	else 
		m_RunOrCrash(component, &Actor::m_OnInitDisabledComponent);
}

void Actor::f_DestroyComponent(Component* component) {
	if (component->ActorBase::BeginDestroy()) {

		if (m_NeedRunComponent(component)) {
			if (isActive())
				m_RunOrCrash(component, &Actor::m_OnDestroyComponent);
			else
				m_RunOrCrash(component, &Actor::m_OnDestroyDisabledComponent);
		}

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

	bool lastActive = isActive();

	transform->friend_ChangeParent(actor);
	m_DeleteFromParent();
	f_parent = actor;
	
	if (f_parent != nullptr)
		f_parent->m_childs.push_back(this);

	if (lastActive != isActive())
		m_OnActiveChanged(!lastActive, true);
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
		m_CrashComponent(component, ex);
	}
}

void Actor::m_RunOrCrash(Component* component, void (Actor::* func)(Component*, Actor*, const Contact&), Actor* otherActor, const Contact& contact) {
	try {
		(this->*func)(component, otherActor, contact);
	}
	catch (std::exception ex) {
		m_CrashComponent(component, ex);
	}
}

void Actor::m_RunOrCrash(Component* component, void (Actor::* func)(Component*, Actor*), Actor* otherActor) {
	try {
		(this->*func)(component, otherActor);
	}
	catch (std::exception ex) {
		m_CrashComponent(component, ex);
	}
}

void Actor::m_CrashComponent(Component* component, std::exception ex) {
	component->f_isCrashed = true;

	auto className = component->GetMeta().name;
	auto actorName = component->actor()->name();
	auto actorId = component->actor()->IdStr();

	std::cout << "ComponentCrash: Component was disabled.\n";
	std::cout << "Component class: " << className << ", ";
	std::cout << "Actor {name: " << actorName << ", ID : " << actorId << "}\n";
	std::cout << "Exception: \n";
	std::cout << ex.what() << "\n";
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

void Actor::m_OnInitDisabledComponent(Component* component) {
	if (!m_NeedRunComponent(component))
		return;

	component->OnInitDisabled();
	component->f_isInited = true;
}

void Actor::m_OnStartComponent(Component* component) {
	assert(component->f_isInited);
	
	m_OnActivateComponent(component);

	assert(component->f_isActive);

	component->OnStart();
	component->f_isStarted = true;
	if (component->csRef().value > 0 && component->f_callbacks.onStart != nullptr) {
		auto method = component->f_callbacks.onStart;
		auto runOrCrash = game()->callbacks().runOrCrush;

		component->f_isCrashed |= runOrCrash(component->csRef(), method);
	}
}

void Actor::m_OnUpdateComponent(Component* component) {
	assert(component->f_isActive);
	assert(component->f_isStarted);

	component->OnUpdate();
	if (component->csRef().value > 0 && component->f_callbacks.onUpdate != nullptr) {
		auto method = component->f_callbacks.onUpdate;
		auto runOrCrash = game()->callbacks().runOrCrush;

		component->f_isCrashed |= runOrCrash(component->csRef(), method);
	}
}

inline void Actor::m_OnFixedUpdateComponent(Component* component) {
	assert(component->f_isActive);
	assert(component->f_isStarted);

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

	if (component->f_isActive)
		m_OnDeactivateComponent(component);

	assert(!component->f_isActive);

	component->OnDestroy();
	if (component->csRef().value > 0 && component->f_callbacks.onDestroy != nullptr) {
		auto method = component->f_callbacks.onDestroy;
		auto runOrCrash = game()->callbacks().runOrCrush;

		component->f_isCrashed |= runOrCrash(component->csRef(), method);
	}
}

void Actor::m_OnDestroyDisabledComponent(Component* component) {
	if (!component->f_isInited)
		return;

	assert(!component->f_isActive);

	component->OnDestroyDisabled();
}

void Actor::m_OnActivateComponent(Component* component) {
	assert(component->f_isInited);
	assert(!component->f_isActive);

	component->OnActivate();
	component->f_isActive = true;
	if (component->csRef().value > 0 && component->f_callbacks.onActivate != nullptr) {
		auto method = component->f_callbacks.onActivate;
		auto runOrCrash = game()->callbacks().runOrCrush;

		component->f_isCrashed |= runOrCrash(component->csRef(), method);
	}
}

void Actor::m_OnDeactivateComponent(Component* component) {
	assert(component->f_isInited);
	assert(component->f_isActive);

	component->OnDeactivate();
	component->f_isActive = false;
	if (component->csRef().value > 0 && component->f_callbacks.onDeactivate != nullptr) {
		auto method = component->f_callbacks.onDeactivate;
		auto runOrCrash = game()->callbacks().runOrCrush;

		component->f_isCrashed |= runOrCrash(component->csRef(), method);
	}
}

void Actor::m_OnCollisionEnterComponent(Component* component, Actor* otherActor, const Contact& contact) {
	if (!component->f_isStarted)
		return;

	component->OnCollisionEnter(otherActor, contact);
	if (component->csRef().value > 0 && component->f_callbacks.onCollisionEnter != nullptr) {
		auto method = component->f_callbacks.onCollisionEnter;
		auto runOrCrash = game()->callbacks().runOrCrushContactEnter;

		component->f_isCrashed |= runOrCrash(component->csRef(), method, otherActor->csRef(), &contact);
	}
}
void Actor::m_OnCollisionExitComponent(Component* component, Actor* otherActor) {
	if (!component->f_isStarted)
		return;

	component->OnCollisionExit(otherActor);
	if (component->csRef().value > 0 && component->f_callbacks.onCollisionExit != nullptr) {
		auto method = component->f_callbacks.onCollisionExit;
		auto runOrCrash = game()->callbacks().runOrCrushContactExit;

		component->f_isCrashed |= runOrCrash(component->csRef(), method, otherActor->csRef());
	}
}

void Actor::m_OnTriggerEnterComponent(Component* component, Actor* otherActor, const Contact& contact) {
	if (!component->f_isStarted)
		return;

	component->OnTriggerEnter(otherActor, contact);
	if (component->csRef().value > 0 && component->f_callbacks.onTriggerEnter != nullptr) {
		auto method = component->f_callbacks.onTriggerEnter;
		auto runOrCrash = game()->callbacks().runOrCrushContactEnter;

		component->f_isCrashed |= runOrCrash(component->csRef(), method, otherActor->csRef(), &contact);
	}
}

void Actor::m_OnTriggerExitComponent(Component* component, Actor* otherActor) {
	if (!component->f_isStarted)
		return;

	component->OnTriggerExit(otherActor);
	if (component->csRef().value > 0 && component->f_callbacks.onTriggerExit != nullptr) {
		auto method = component->f_callbacks.onTriggerExit;
		auto runOrCrash = game()->callbacks().runOrCrushContactExit;

		component->f_isCrashed |= runOrCrash(component->csRef(), method, otherActor->csRef());
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
	auto parent = CppRefs::GetPointer<Actor>(newObjRef);
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
	auto comp = CppRefs::ThrowPointer<Component>(compRef);
	comp->Destroy();
}

DEF_FUNC(Actor, Destroy, void)(CppRef objRef) {
	auto actor = CppRefs::ThrowPointer<Actor>(objRef);
	actor->Destroy();
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

DEF_PROP_GETSET_F(Actor, size_t, flags, flags);

DEF_PROP_GETSET_STR(Actor, name);
DEF_PROP_GETSET_STR(Actor, prefabId);

DEF_PROP_GET(Actor, bool, isActive);
DEF_PROP_GETSET(Actor, bool, activeSelf);

DEF_PROP_GETSET(Actor, Vector3, localPosition)
DEF_PROP_GETSET(Actor, Vector3, localRotation)
DEF_PROP_GETSET(Actor, Quaternion, localRotationQ)
DEF_PROP_GETSET(Actor, Vector3, localScale)

DEF_PROP_GETSET(Actor, Vector3, worldPosition)
DEF_PROP_GETSET(Actor, Quaternion, worldRotationQ)
DEF_PROP_GETSET(Actor, Vector3, worldScale)

DEF_PROP_GETSET_F(Component, bool, runtimeOnly, runtimeOnly);
DEF_PROP_GETSET_F(Component, bool, f_isCrashed, f_isCrashed);
DEF_PROP_GET(Component, bool, IsActivated);

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