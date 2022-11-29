#pragma once
#include "CSBridge.h"


template<typename TComponent, typename>
size_t GameObject::CreateComponent(size_t csRef) {
	Component* component = new TComponent();

	component->f_ref = Refs::Create(component);
	component->f_cppRef = component->f_ref.id();
	component->f_csRef = csRef;

	std::cout << "+: GameObject( " << this->csRef() << ", " << cppRef() << ").CreateComponent() -> ( " << component->f_csRef << ",  " << component->f_cppRef << ")" << std::endl;

	return component->f_cppRef;
}

template<IsCppComponent TComponent, typename>
TComponent* GameObject::AddComponentCs() {
	std::cout << "+: GameObject(" << csRef() << ", " << cppRef() << ").AddComponentCs()" << std::endl;

	return AddComponentCs<TComponent>(TComponent::meta_csComponentName);
}

template<typename TComponent, typename>
TComponent* GameObject::AddComponentCs(const std::string& className) {
	std::cout << "+: GameObject(" << csRef() << ", " << cppRef() << ").AddComponentCs(" << className << ")" << std::endl;

	auto cppCompRef = mono_AddComponent(this->csRef(), (size_t)className.c_str(), className.size());

	return Refs::ThrowPointer<TComponent>(cppCompRef);
}

template<typename TComponent, typename>
TComponent* GameObject::AddComponent() {
	Component* component = new TComponent();
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