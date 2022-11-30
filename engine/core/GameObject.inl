#pragma once
#include "CSBridge.h"



template<typename TComponent, typename>
CppRef GameObject::CreateComponent(CsRef csRef) {
	Component* component = new TComponent();

	component->f_ref = CppRefs::Create(component);
	component->f_cppRef = component->f_ref.id();
	component->f_csRef = csRef;

	std::cout << "+: GameObject(" << this->csRef() << ", " << cppRef() << ").CreateComponent() -> (" << component->f_csRef << ",  " << component->f_cppRef << ")" << std::endl;

	return component->f_cppRef;
}

template<HasCsMetaData TComponent, typename>
TComponent* GameObject::AddComponent() {

	if (IS_PURE_COMPONENT(TComponent))
		return m_AddPureCppComponent<TComponent>();

	return m_AddCsCppComponent<TComponent>();
}

template<typename TComponent, typename>
TComponent* GameObject::m_AddPureCppComponent() {
	std::cout << "+: GameObject(" << csRef() << ", " << cppRef() << ").m_AddPureCppComponent()" << std::endl;

	auto cppCompRef = CreateComponent<TComponent>(RefCs(0));
	Component* component = CppRefs::ThrowPointer<TComponent>(cppCompRef);
	m_InitComponent(component);
	return (TComponent*)component;
}

template<typename TComponent, typename>
TComponent* GameObject::m_AddCsCppComponent() {
	std::cout << "+: GameObject(" << csRef() << ", " << cppRef() << ").m_AddCsCppComponent()" << std::endl;

	return AddCsComponent<TComponent>(TComponent::meta_csComponentName);
}


template<typename TComponent, typename>
TComponent* GameObject::AddCsComponent(const std::string& className) {
	std::cout << "+: GameObject(" << csRef() << ", " << cppRef() << ").AddComponentCs(" << className << ")" << std::endl;

	auto cppCompRef = mono_AddComponent(this->csRef(), (size_t)className.c_str(), className.size());

	return CppRefs::ThrowPointer<TComponent>(cppCompRef);
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