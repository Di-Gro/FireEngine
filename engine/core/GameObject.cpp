#include "GameObject.h"

#include "SimpleMath.h"

#include "Game.h"
#include "Math.h"
#include "CameraComponent.h"

/// 
/// В Update компоненты МОГУТ обновлять позиции GameObject-а. 
/// Перед вызовом OnDraw компонента, нужно рекурсивно рассчитать 
/// матрицы трансформаций всех родительских объектов сверху вниз.

DEF_OBJECT(GameObject, 0) { }

bool GameObject::mono_inited;

mono::mono_method_invoker<CppRef(CsRef, size_t, size_t)> GameObject::mono_AddComponent;
mono::mono_method_invoker<CsRef(CppRef, CppRef)> GameObject::mono_CreateTransform;
mono::mono_method_invoker<void(CsRef)> GameObject::mono_RemoveTransform;
mono::mono_method_invoker<void(CsRef)> GameObject::mono_OnInit;
mono::mono_method_invoker<void(CsRef)> GameObject::mono_OnStart;
mono::mono_method_invoker<void(CsRef)> GameObject::mono_OnUpdate;
mono::mono_method_invoker<void(CsRef)> GameObject::mono_OnDestroy;


GameObject::GameObject() {  }

GameObject::~GameObject() { }

void GameObject::m_CreateTransform() {
	transform = new Transform();
	transform->friend_gameObject = this;

	transform->f_ref = CppRefs::Create(transform);
	transform->f_cppRef = transform->f_ref.id();

	auto arg_obj = transform->f_cppRef;
	auto arg_class = RefCpp(CppRefs::Create(ClassInfo::Get<Transform>()).id());
	transform->f_csRef = mono_CreateTransform(arg_class, arg_obj);
}

void GameObject::m_RemoveTransform() {
	mono_RemoveTransform(transform->csRef());
	CppRefs::Remove(transform->f_ref);

	delete transform;
}

void GameObject::f_Init(Game* game, std::string name) {
	this->name = name;
	f_game = game;
	friend_gameObject = this;

	m_InitMono();
	m_CreateTransform();
};

void GameObject::m_InitMono() {
	if (mono_inited)
		return;

	auto type = game()->mono()->GetType("Engine", "GameObject");
	mono_AddComponent = mono::make_method_invoker<CppRef(CsRef, size_t, size_t)>(type, "cpp_AddComponent");
	mono_OnInit = mono::make_method_invoker<void(CsRef)>(type, "cpp_InitComponent");
	mono_OnStart = mono::make_method_invoker<void(CsRef)>(type, "cpp_StartComponent");
	mono_OnUpdate = mono::make_method_invoker<void(CsRef)>(type, "cpp_UpdateComponent");
	mono_OnDestroy = mono::make_method_invoker<void(CsRef)>(type, "cpp_DestroyComponent");

	auto trType = game()->mono()->GetType("Engine", "Transform");
	mono_CreateTransform = mono::make_method_invoker<CsRef(CppRef, CppRef)>(trType, "cpp_Create");
	mono_RemoveTransform = mono::make_method_invoker<void(CsRef)>(trType, "cpp_Remove");
	
	mono_inited = true;
}

void GameObject::f_Destroy() {

	for (int i = m_childs.size() - 1; i >= 0; --i)
		m_childs[i]->Destroy();

	assert(m_childs.size() == 0);

	auto it = m_components.begin();
	while (it != m_components.end()) {
		f_DestroyComponent(*it);
		it = m_EraseComponent(it);
	}
	m_DeleteFromParent();
	m_RemoveTransform();

	f_game = nullptr;
	friend_gameObject = nullptr;
}

void GameObject::f_Update() {

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
			m_OnUpdateComponent(component); // component->OnUpdate();
		} else {
			m_OnStartComponent(component); // component->OnStart();
			component->friend_isStarted = true;
		}
		it++;
	}
};

void GameObject::f_Draw() {
	for (auto component : m_components) {
		if (!component->IsDestroyed()) {
			component->OnDraw();

			if (game()->render()->camera()->drawDebug)
				component->OnDrawDebug();
		}
	}
};

void GameObject::f_DrawUI() {
	for (auto component : m_components) {
		if (!component->IsDestroyed())
			component->OnDrawUI();
	}
};

std::list<Component*>::iterator GameObject::m_EraseComponent(std::list<Component*>::iterator it) {
	auto* component = *it;
	auto next = m_components.erase(it);
	delete component;
	return next;
}

void GameObject::m_InitComponent(Component* component) {
	auto it = m_components.insert(m_components.end(), component);

	component->GameObjectBase::friend_gameObject = this;
	component->GameObjectBase::friend_component = component;
	component->GameObjectBase::transform = transform;

	m_OnInitComponent(component); //component->OnInit();
}

void GameObject::f_DestroyComponent(Component* component) {
	if (component->GameObjectBase::friend_CanDestroy()) {
		component->GameObjectBase::friend_StartDestroy();

		m_OnDestroyComponent(component); // component->OnDestroy();

		if (CppRefs::IsValid(component->f_ref)) {
			CppRefs::Remove(component->f_ref);
			std::cout << "+: CppRefs.Remove(): " << component->cppRef() << std::endl;
		}
		component->GameObjectBase::friend_gameObject = nullptr;
		component->GameObjectBase::friend_component = nullptr;
	}
}

void GameObject::f_SetParent(GameObject* gameObject) {
	if (f_parent == gameObject)
		return;

	transform->friend_ChangeParent(gameObject);
	m_DeleteFromParent();
	f_parent = gameObject;

	if (f_parent != nullptr)
		f_parent->m_childs.push_back(this);
}

void GameObject::m_DeleteFromParent() {
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

//int GameObject::GetСhildrenCount() {
//	int count = 0;
//
//	for (auto child : m_childs) {
//		if (!child->IsDestroyed() && child->csRef() > 0) {
//			count++;
//		}
//	}
//	return count;
//}

int GameObject::GetComponentsCount() {
	int count = 0;

	for (auto component : m_components) {
		if (!component->IsDestroyed() && component->csRef() > 0) {
			count++;
		}
	}
	return count;
}

void GameObject::WriteComponentsRefs(size_t* csRefsList) {
	size_t* ptr = csRefsList;

	for (auto component : m_components) {
		if (!component->IsDestroyed() && component->csRef() > 0) {
			*ptr = component->csRef();
			ptr++;
		}
	}
}

GameObject* GameObject::GetChild(int index) {
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
	//str += "local scale:    " + ToString(transform.scale) + "\n";

	str += "world position: " + ToString(transform.worldPosition()) + "\n";
	str += "world rotation: " + ToString(deg(transform.worldRotation())) + "\n";
	//str += "world scale:    " + ToString(transform.worldScale()) + "\n";

	return str;
}

void GameObject::RecieveGameMessage(const std::string& msg) {

	if (msg == "tr") {
		std::cout << "Object: " << name << " (" << f_objectID << ")" << std::endl;
		std::cout << "transform:" << std::endl;
		std::cout << ToString(*transform) << std::endl;
		std::cout << std::endl;
	}
	else if (msg == "tr.lpos") {
		std::cout << "Object: " << name << " (" << f_objectID << ")" << std::endl;
		std::cout << "local position: ";
		std::cout << ToString(transform->localPosition()) << std::endl;
		std::cout << std::endl;
	} 
	else if (msg == "tr.lrot") {
		std::cout << "Object: " << name << " (" << f_objectID << ")" << std::endl;
		std::cout << "local rotation: ";
		std::cout << ToString(deg(transform->localRotation())) << std::endl;
		std::cout << std::endl;
	}

	for (auto component : m_components) {
		if (!component->IsDestroyed())
			component->RecieveGameMessage(msg);
	}
}

void GameObject::AddComponentByRef(CppRef cppComponentRef) {
	std::cout << "+: GameObject(" << csRef() << ", " << cppRef() << ").AddComponentByRef(): " << cppComponentRef << std::endl;

	auto* component = CppRefs::ThrowPointer<Component>(cppComponentRef);
	if (component != nullptr) {
		m_InitComponent(component);
	}
	else {
		std::cout << "+: GameObject.AddComponentByRef(): NULL component" << std::endl;
	}
}

void GameObject::m_OnInitComponent(Component* component) {
	component->OnInit();
	if (component->csRef().value > 0)
		mono_OnInit(component->csRef());
}

void GameObject::m_OnStartComponent(Component* component) {
	component->OnStart();
	if (component->csRef().value > 0)
		mono_OnStart(component->csRef());
}

void GameObject::m_OnUpdateComponent(Component* component) {
	component->OnUpdate();
	if (component->csRef().value > 0)
		mono_OnUpdate(component->csRef());
}

void GameObject::m_OnDestroyComponent(Component* component) {
	component->OnDestroy();
	if (component->csRef().value > 0)
		mono_OnDestroy(component->csRef());
}


DEF_FUNC(GameObject, gameObject_get, CsRef)(CppRef objBaseRef) {
	return CppRefs::ThrowPointer<GameObjectBase>(objBaseRef)->gameObject()->csRef();
}

DEF_FUNC(GameObject, parent_get, CsRef)(CppRef objRef) {
	auto parent = CppRefs::ThrowPointer<GameObject>(objRef)->GetParent();
	return parent != nullptr ? parent->csRef() : CsRef();
}

DEF_FUNC(GameObject, parent_set, void)(CppRef objRef, CppRef newObjRef) {
	std::cout << "+: GameObject_parent_set(): " << objRef << " -> " << newObjRef << std::endl;

	auto object = CppRefs::ThrowPointer<GameObject>(objRef);
	auto parent = CppRefs::ThrowPointer<GameObject>(newObjRef);
	object->SetParent(parent);
}

DEF_FUNC(GameObject, AddComponentByRef, void)(CppRef objRef, CppRef compRef) {
	CppRefs::ThrowPointer<GameObject>(objRef)->AddComponentByRef(compRef);
}

DEF_FUNC(GameObject, DestroyComponent, void)(CppRef compRef) {
	CppRefs::ThrowPointer<Component>(compRef)->Destroy();
}

DEF_FUNC(GameObject, Destroy, void)(CppRef objRef) {
	CppRefs::ThrowPointer<GameObject>(objRef)->Destroy();
}

DEF_FUNC(GameObject, GetComponentsCount, int)(CppRef objRef) {
	return CppRefs::ThrowPointer<GameObject>(objRef)->GetComponentsCount();
}

DEF_FUNC(GameObject, WriteComponentsRefs, void)(CppRef objRef, size_t listPtr) {
	CppRefs::ThrowPointer<GameObject>(objRef)->WriteComponentsRefs((size_t*)listPtr);
}

DEF_FUNC(GameObject, GetChildrenCount, int)(CppRef objRef) {
	return CppRefs::ThrowPointer<GameObject>(objRef)->GetChildrenCount();
}

DEF_FUNC(GameObject, GetChild, CsRef)(CppRef objRef, int index) {
	auto child = CppRefs::ThrowPointer<GameObject>(objRef)->GetChild(index);
	return child != nullptr ? child->csRef() : CsRef();
}