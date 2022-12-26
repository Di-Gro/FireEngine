#pragma once
#include "CSBridge.h"
#include "ComponentMeta.h"

template<typename TComponent, typename>
TComponent* Actor::inner_CreateComponent(CsRef csRef) {
	Component* component = new TComponent();

	component->f_ref = CppRefs::Create(component);
	component->f_cppRef = component->f_ref.cppRef();
	component->f_csRef = csRef;

	//auto className = component->GetMeta().name;
	//std::cout << "+: Actor(" << this->csRef() << ", " << this->cppRef() << ").inner_CreateComponent<" << className << ">() -> (" << component->csRef() << ",  " << component->cppRef() << ")" << std::endl;

	return (TComponent*)component;
}

template<IsCppAddableComponent TComponent, typename >
TComponent* Actor::AddComponent() {
	Component* component = inner_CreateComponent<TComponent>();
	auto meta = component->GetMeta();

	//std::cout << "+: Actor(" << this->csRef() << ", " << this->cppRef() << ").AddComponent<\"" << meta.name << "\">() ->" << std::endl;

	if (!meta.isPure) {
		CppObjectInfo info;
		info.cppRef = component->cppRef();
		info.classRef = meta.classInfoRef;

		auto csCompRef = mono_AddComponent(this->csRef(), (size_t)meta.name, std::strlen(meta.name), info);
		component->f_csRef = csCompRef;
	}
	m_BindComponent(component);
	m_InitComponent(component);

	//std::cout << "+: -> " << component->csRef() << ", " << component->cppRef() << std::endl;
	return (TComponent*)component;
}

template<typename TComponent, typename>
TComponent* Actor::AddComponent(const std::string& className) {
	//std::cout << "+: Actor(" << csRef() << ", " << cppRef() << ").AddComponent(" << className << ")" << std::endl;

	auto cppCompRef = mono_AddCsComponent(this->csRef(), (size_t)className.c_str(), className.size());

	return CppRefs::ThrowPointer<TComponent>(cppCompRef);
}

template<typename TComponent, typename>
TComponent* Actor::GetComponent() {
	TComponent* ptr = nullptr;
	for (auto component : m_components) {
		ptr = dynamic_cast<TComponent*>(component);
		if (ptr != nullptr)
			break;
	}
	return ptr;
} 

template<typename TComponent, typename>
TComponent* Actor::GetComponentInChild() {
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