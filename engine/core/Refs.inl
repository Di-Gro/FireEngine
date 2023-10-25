#include "Refs.h"

#include <iostream>
#include <string>


template<typename T>
T* CppRefs::GetPointer(const Ref2& ref) {
	if (!ref.isInitialized())
		return nullptr;

	return CppRefs::GetPointer<T>(ref.cppRef());
}

template<typename T>
T* CppRefs::GetPointer(CppRef refId) {
	auto it = m_idMap.find(refId);

	if (it != m_idMap.end())
		return (T*) it->second;

	return nullptr;
}

template<typename T>
T* CppRefs::ThrowPointer(CppRef refId) {
	auto* pointer = CppRefs::GetPointer<T>(refId);
	if (pointer != nullptr)
		return pointer;

	auto text = "CppRefs::ThrowPointer(): CppRef:" + std::to_string(refId.value) + " not exists. Maybe object was destroyed.";
	throw std::exception(text.c_str());
}