#include "Actor.h"

#include "SimpleMath.h"

#include "Game.h"
#include "Math.h"
#include "CameraComponent.h"

/// 
/// В Update компоненты МОГУТ обновлять позиции Actor-а. 
/// Перед вызовом OnDraw компонента, нужно рекурсивно рассчитать 
/// матрицы трансформаций всех родительских объектов сверху вниз.

DEF_OBJECT(Actor, 0) { }

bool Actor::mono_inited;

mono::mono_method_invoker<CsRef(CsRef, size_t, size_t, CppObjectInfo)> Actor::mono_AddComponent;
mono::mono_method_invoker<CppRef(CsRef, size_t, size_t)> Actor::mono_AddCsComponent;
mono::mono_method_invoker<void(CsRef, size_t, size_t)> Actor::mono_SetName;


Actor::Actor() {  }

Actor::~Actor() { }

void Actor::m_CreateTransform() {
	transform = &m_transform;
	transform->friend_gameObject = this;
}

//void Actor::m_RemoveTransform() {
//}

void Actor::f_Init(Game* game) {
	f_game = game;
	friend_gameObject = this;

	m_InitMono();
	m_CreateTransform();
};

void Actor::m_InitMono() {
	if (mono_inited)
		return;

	auto type = game()->mono()->GetType("Engine", "Actor");
	mono_AddComponent = mono::make_method_invoker<CsRef(CsRef, size_t, size_t, CppObjectInfo)>(type, "cpp_AddComponent");
	mono_AddCsComponent = mono::make_method_invoker<CppRef(CsRef, size_t, size_t)>(type, "cpp_AddCsComponent");
	mono_SetName = mono::make_method_invoker<void(CsRef, size_t, size_t)>(type, "cpp_SetName");

	mono_inited = true;
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
	//m_RemoveTransform();

	f_game = nullptr;
	friend_gameObject = nullptr;
}

void Actor::f_Update() {

	auto it = m_components.begin();
	while (it != m_components.end()) {

		auto component = (*it);
		if (component->IsDestroyed()) {
			component->friend_timeToDestroy--;
			if (component->friend_timeToDestroy <= 0)
				it = m_EraseComponent(it);
			continue;
		}
		if (component->friend_isStarted) {
			m_OnUpdateComponent(component); 
		} else {
			m_OnStartComponent(component);
			component->friend_isStarted = true;
		}
		it++;
	}
};

void Actor::f_Draw() {
	for (auto component : m_components) {
		if (!component->IsDestroyed()) {
			component->OnDraw();

			if (game()->render()->camera()->drawDebug)
				component->OnDrawDebug();
		}
	}
};

void Actor::f_DrawUI() {
	for (auto component : m_components) {
		if (!component->IsDestroyed())
			component->OnDrawUI();
	}
};

std::list<Component*>::iterator Actor::m_EraseComponent(std::list<Component*>::iterator it) {
	auto* component = *it;
	auto next = m_components.erase(it);
	delete component;
	return next;
}

void Actor::m_InitComponent(Component* component) {
	auto it = m_components.insert(m_components.end(), component);

	component->ActorBase::friend_gameObject = this;
	component->ActorBase::friend_component = component;
	component->ActorBase::transform = transform;

	m_OnInitComponent(component); 
}

void Actor::f_DestroyComponent(Component* component) {
	if (component->ActorBase::friend_CanDestroy()) {
		component->ActorBase::friend_StartDestroy();

		m_OnDestroyComponent(component);

		if (CppRefs::IsValid(component->f_ref)) {
			CppRefs::Remove(component->f_ref);
			//std::cout << "+: CppRefs.Remove(): " << component->cppRef() << std::endl;
		}
		component->ActorBase::friend_gameObject = nullptr;
		component->ActorBase::friend_component = nullptr;
	}
}

void Actor::f_SetParent(Actor* actor) {
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

void Actor::SetName(const std::string& value) {
	mono_SetName(csRef(), (size_t)value.c_str(), value.size());
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

void Actor::RecieveGameMessage(const std::string& msg) {

	//if (msg == "tr") {
	//	std::cout << "Object: " << name << " (" << f_actorID << ")" << std::endl;
	//	std::cout << "transform:" << std::endl;
	//	std::cout << ToString(*transform) << std::endl;
	//	std::cout << std::endl;
	//}
	//else if (msg == "tr.lpos") {
	//	std::cout << "Object: " << name << " (" << f_actorID << ")" << std::endl;
	//	std::cout << "local position: ";
	//	std::cout << ToString(localPosition()) << std::endl;
	//	std::cout << std::endl;
	//} 
	//else if (msg == "tr.lrot") {
	//	std::cout << "Object: " << name << " (" << f_actorID << ")" << std::endl;
	//	std::cout << "local rotation: ";
	//	std::cout << ToString(deg(localRotation())) << std::endl;
	//	std::cout << std::endl;
	//}

	//for (auto component : m_components) {
	//	if (!component->IsDestroyed())
	//		component->RecieveGameMessage(msg);
	//}
}

void Actor::m_OnInitComponent(Component* component) {
	component->OnInit();
	if (component->csRef().value > 0 && component->m_callbacks.onInit != nullptr)
		component->m_callbacks.onInit();
}

void Actor::m_OnStartComponent(Component* component) {
	component->OnStart();
	if (component->csRef().value > 0 && component->m_callbacks.onStart != nullptr)
		component->m_callbacks.onStart();
}

void Actor::m_OnUpdateComponent(Component* component) {
	component->OnUpdate();
	if (component->csRef().value > 0 && component->m_callbacks.onUpdate != nullptr)
		component->m_callbacks.onUpdate();
}

void Actor::m_OnDestroyComponent(Component* component) {
	component->OnDestroy();
	if (component->csRef().value > 0 && component->m_callbacks.onDestroy != nullptr)
		component->m_callbacks.onDestroy();
}

void Actor::m_SetComponentCallbacks(Component* component, ComponentCallbacks callbacks) {
	component->m_callbacks = callbacks;
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

DEF_PROP_GETSET(Actor, Vector3, localPosition)
DEF_PROP_GETSET(Actor, Vector3, localRotation)
DEF_PROP_GETSET(Actor, Quaternion, localRotationQ)
DEF_PROP_GETSET(Actor, Vector3, localScale)

DEF_PROP_GETSET(Actor, Vector3, worldPosition)
DEF_PROP_GETSET(Actor, Quaternion, worldRotationQ)
DEF_PROP_GETSET(Actor, Vector3, worldScale)

DEF_PROP_GET(Actor, Vector3, localForward)
DEF_PROP_GET(Actor, Vector3, localUp)
DEF_PROP_GET(Actor, Vector3, localRight)

DEF_PROP_GET(Actor, Vector3, forward)
DEF_PROP_GET(Actor, Vector3, up)
DEF_PROP_GET(Actor, Vector3, right)

DEF_FUNC(Actor, SetComponentCallbacks, void)(CppRef componentRef, const ComponentCallbacks& callbacks) {
	auto component = CppRefs::ThrowPointer<Component>(componentRef);
	component->actor()->m_SetComponentCallbacks(component, callbacks);
}