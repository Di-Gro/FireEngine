#include "Refs.h"

std::unordered_map<size_t, void*> CppRefs::m_idMap = std::unordered_map<size_t, void*>();
std::unordered_map<void*, size_t> CppRefs::m_ptrMap = std::unordered_map<void*, size_t>();

size_t CppRefs::m_nextId = 1;

CppRef CppRefs::GetRef(void* ptr) {
	if (!m_ptrMap.contains((void*)ptr))
		return RefCpp(0);

	return RefCpp(m_ptrMap.at(ptr));
}

bool CppRefs::IsValidPointer(const void* ptr) {
	return m_ptrMap.contains((void *)ptr);
}

bool CppRefs::IsValid(const CppRef& ref) {
	return m_idMap.contains(ref);
}

bool CppRefs::IsValid(const Ref2& ref) {
	return ref.isInitialized() && m_idMap.contains(ref.cppRef());
}

bool CppRefs::IsRemoved(const Ref2& ref) {
	return ref.isInitialized() && !m_idMap.contains(ref.cppRef());
}

Ref2 CppRefs::Create(const Ref2& ref) {
	return Ref2(ref.cppRef());
}

Ref2 CppRefs::Create(void* ptr) {
	auto it = m_ptrMap.find(ptr);

	if (it != m_ptrMap.end())
		return Ref2(it->second);

	m_idMap[m_nextId] = ptr;
	m_ptrMap[ptr] = m_nextId;
	return Ref2(m_nextId++);
}

void CppRefs::Reset(Ref2& ref, void* ptr) {

	if (!ref.isInitialized()) {
		ref = CppRefs::Create(ptr);
		return;
	}
	auto it = m_idMap.find(ref.cppRef());

	if (it != m_idMap.end())
		m_ptrMap.erase(it->second);

	m_idMap[ref.cppRef()] = ptr;
	m_ptrMap[ptr] = ref.cppRef();
}

void CppRefs::Reset(void* oldPtr, void* newPtr) {
	auto it = m_ptrMap.find(oldPtr);

	if (it != m_ptrMap.end()) {
		int id = it->second;

		m_ptrMap.erase(oldPtr);

		m_idMap[id] = newPtr;
		m_ptrMap[newPtr] = id;
	}
}

void CppRefs::Remove(const Ref2& ref) {
	if (!ref.isInitialized())
		return;

	auto it = m_idMap.find(ref.cppRef());

	if (it != m_idMap.end()) {
		m_ptrMap.erase(it->second);
		m_idMap.erase(it->first);
	}
}