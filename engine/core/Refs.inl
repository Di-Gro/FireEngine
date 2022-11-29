#include "Refs.h"

#include <iostream>


template<typename T>
T* Refs::GetPointer(const Ref2& ref) {
	if (!ref.isInitialized())
		return nullptr;

	return Refs::GetPointer<T>(ref.id());
}

template<typename T>
T* Refs::GetPointer(CppRef refId) {
	auto it = m_idMap.find(refId);

	if (it != m_idMap.end())
		return (T*) it->second;

	return nullptr;
}

template<typename T>
T* Refs::ThrowPointer(CppRef refId) {
	auto* pointer = Refs::GetPointer<T>(refId);
	if (pointer != nullptr)
		return pointer;

	std::cout << "+: Refs::ThrowPointer(): NullptrException" << std::endl;
	throw std::exception("+: Refs::ThrowPointer(): NullptrException");
}