#pragma once

#include <vector>
#include <unordered_map>
#include <mono_game_types.h>


class Ref2 {
	friend class CppRefs;

private:
	size_t m_refId = 0;

public:

	size_t id() const { return m_refId; }
	bool isInitialized() const { return m_refId > 0; }

	Ref2() { }
	Ref2(const Ref2& ref) { m_refId = ref.m_refId; }

	Ref2& operator=(const Ref2& ref) { 
		m_refId = ref.m_refId; 
		return *this;
	}

private:
	
	Ref2(size_t refId) { m_refId = refId; }

};


class CppRefs {
private:
	static std::unordered_map<size_t, void*> m_idMap;
	static std::unordered_map<void*, size_t> m_ptrMap;

	static size_t m_nextId; // default = 1

public:

	static bool IsValid(const Ref2& ref);
	static bool IsRemoved(const Ref2& ref);

	static Ref2 Create(const Ref2& ref);
	static Ref2 Create(void* ptr);

	static void Reset(Ref2& ref, void* ptr);
	static void Reset(void* oldPtr, void* newPtr);

	static void Remove(const Ref2& ref);

	template<typename T>
	static T* GetPointer(const Ref2& ref);

	template<typename T>
	static T* GetPointer(CppRef refId);

	template<typename T>
	static T* ThrowPointer(CppRef refId);

};

#include "Refs.inl"