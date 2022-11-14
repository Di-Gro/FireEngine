#pragma once

template<typename TComponent, typename>
TComponent* GameObject::AddComponent() {
	Component* component = new TComponent(this);
	m_InitComponent(component);
	return (TComponent*)component;
}

template<typename TComponent, typename>
TComponent* GameObject::GetComponent() {
	TComponent* ptr = nullptr;
	for (auto component : m_components) {
		ptr = dynamic_cast<TComponent*>(component);
		if (ptr != nullptr)
			break;
	}
	return ptr;
} 

template<typename TComponent, typename>
TComponent* GameObject::GetComponentInChild() {
	auto* ptr = GetComponent<TComponent>();
	if (ptr != nullptr)
		return ptr;

	for (auto child : m_childs) {
		ptr = child->GetComponentInChild<TComponent>();
		if (ptr != nullptr)
			return ptr;
	}
	return nullptr;
}