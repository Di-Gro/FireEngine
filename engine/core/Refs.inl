#include "Refs.h"

#include <iostream>


template<typename T>
T* CppRefs::GetPointer(const Ref2& ref) {
	if (!ref.isInitialized())
		return nullptr;

	return CppRefs::GetPointer<T>(ref.id());
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

	std::cout << "+: CppRefs::ThrowPointer(): NullptrException" << std::endl;
	throw std::exception("+: CppRefs::ThrowPointer(): NullptrException");
}