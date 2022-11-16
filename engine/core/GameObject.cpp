#include "GameObject.h"

#include "SimpleMath.h"

#include "Game.h"
#include "Math.h"
#include "CameraComponent.h"

/// 
/// В Update компоненты МОГУТ обновлять позиции GameObject-а. 
/// Перед вызовом OnDraw компонента, нужно рекурсивно рассчитать 
/// матрицы трансформаций всех родительских объектов сверху вниз.
/// 

GameObject::GameObject() : GameObjectBase(this) { }

void GameObject::f_Init(Game* game, std::string name) {
	transform.friend_gameObject = this;
	this->name = name;
	friend_game = game;
	friend_gameObject = this;
};

void GameObject::f_Destroy() {

	//if (name == "camera root")
	//	int wtf = 9;

	for (int i = m_childs.size() - 1; i >= 0; --i)
		m_childs[i]->Destroy();

	assert(m_childs.size() == 0);
	//m_childs.clear();

	auto it = m_components.begin();
	while (it != m_components.end()) {
		f_DestroyComponent(*it);
		it = m_EraseComponent(it);
	}
	m_DeleteFromParent();
	friend_game = nullptr;
	friend_gameObject = nullptr;
}

void GameObject::f_Update() {
	//transform.friend_dirty = true;

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
			component->OnUpdate();
		} else {
			component->OnStart();
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

	component->OnInit();
}

void GameObject::f_DestroyComponent(Component* component) {
	if (component->GameObjectBase::friend_CanDestroy()) {
		component->GameObjectBase::friend_StartDestroy();

		component->OnDestroy();

		component->GameObjectBase::friend_gameObject = nullptr;
		component->GameObjectBase::friend_component = nullptr;
	}
}

void GameObject::f_SetParent(GameObject* gameObject) {
	if (friend_parent == gameObject)
		return;

	transform.friend_ChangeParent(gameObject);
	m_DeleteFromParent();
	friend_parent = gameObject;

	if (friend_parent != nullptr)
		friend_parent->m_childs.push_back(this);
}

void GameObject::m_DeleteFromParent() {
	if (friend_parent != nullptr) {
		for (int i = friend_parent->m_childs.size() - 1; i >= 0; --i) {
			if (friend_parent->m_childs[i] == this) {
				friend_parent->m_childs.erase(friend_parent->m_childs.begin() + i);
				break;
			}
		}
		friend_parent = nullptr;
	}
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
		std::cout << "Object: " << name << " (" << friend_objectID << ")" << std::endl;
		std::cout << "transform:" << std::endl;
		std::cout << ToString(transform) << std::endl;
		std::cout << std::endl;
	}
	else if (msg == "tr.lpos") {
		std::cout << "Object: " << name << " (" << friend_objectID << ")" << std::endl;
		std::cout << "local position: ";
		std::cout << ToString(transform.localPosition()) << std::endl;
		std::cout << std::endl;
	} 
	else if (msg == "tr.lrot") {
		std::cout << "Object: " << name << " (" << friend_objectID << ")" << std::endl;
		std::cout << "local rotation: ";
		std::cout << ToString(deg(transform.localRotation())) << std::endl;
		std::cout << std::endl;
	}

	for (auto component : m_components) {
		if (!component->IsDestroyed())
			component->RecieveGameMessage(msg);
	}
}