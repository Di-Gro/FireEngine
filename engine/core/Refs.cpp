#include "Refs.h"

//std::vector<void*> Refs::m_refs = std::vector<void*>();
std::unordered_map<size_t, void*> Refs::m_idMap = std::unordered_map<size_t, void*>();
std::unordered_map<void*, size_t> Refs::m_ptrMap = std::unordered_map<void*, size_t>();

size_t Refs::m_nextId = 1;


bool Refs::IsValid(const Ref2& ref) {
	return ref.isInitialized() && m_idMap.contains(ref.id());
}

bool Refs::IsRemoved(const Ref2& ref) {
	return ref.isInitialized() && !m_idMap.contains(ref.id());
}

Ref2 Refs::Create(const Ref2& ref) {
	return Ref2(ref.id());
}

Ref2 Refs::Create(void* ptr) {
	auto it = m_ptrMap.find(ptr);

	if (it != m_ptrMap.end())
		return Ref2(it->second);

	m_idMap[m_nextId] = ptr;
	m_ptrMap[ptr] = m_nextId;
	return Ref2(m_nextId++);
}

void Refs::Reset(Ref2& ref, void* ptr) {

	if (!ref.isInitialized()) {
		ref = Refs::Create(ptr);
		return;
	}
	auto it = m_idMap.find(ref.id());

	if (it != m_idMap.end())
		m_ptrMap.erase(it->second);

	m_idMap[ref.id()] = ptr;
	m_ptrMap[ptr] = ref.id();
}

void Refs::Reset(void* oldPtr, void* newPtr) {
	auto it = m_ptrMap.find(oldPtr);

	if (it != m_ptrMap.end()) {
		int id = it->second;

		m_ptrMap.erase(oldPtr);

		m_idMap[id] = newPtr;
		m_ptrMap[newPtr] = id;
	}
}

void Refs::Remove(const Ref2& ref) {
	if (!ref.isInitialized())
		return;

	auto it = m_idMap.find(ref.id());

	if (it != m_idMap.end()) {
		m_ptrMap.erase(it->second);
		m_idMap.erase(it->first);
	}
}