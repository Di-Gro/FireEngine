#pragma once

#include <vector>

class Refs {
private:
	static std::vector<void*> m_refs;

public:
	template<typename T>
	static T* GetRef(size_t refId) {
		if (refId >= 0 && refId < m_refs.size()) {
			return (T*)m_refs.at(refId);
		}
		else {
			// Выбросить исключение в C#, что объект разрушен
			return nullptr;
		}
	}

	static size_t CreateRef(void* ptr) {
		m_refs.push_back(ptr);
		return m_refs.size() - 1;
	}

	static void RemoveRef(size_t refId) {
		if (refId >= 0 && refId < m_refs.size())
			m_refs[refId] = nullptr;
	}
};

template<typename T>
class Ref {
public:
	T obj;

private:
	size_t m_refId;

public:
	size_t RefId() { return m_refId; }

	Ref() { m_refId = Refs::CreateRef(&obj); }
	~Ref() { Refs::RemoveRef(m_refId); }
};
