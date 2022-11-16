#pragma once
#include <map>

#include "Ref.h"
#include "MonoInst.h"

#include "monopp/mono_jit.h"
#include "monopp/mono_domain.h"
#include "monopp/mono_assembly.h"
#include "monopp/mono_type.h"
#include "monopp/mono_method_invoker.h"


template<typename T>
class CSLinked {
private:
	Ref<T> m_ref;

	MonoInst* m_mono;

	std::string m_namespace;
	std::string m_name;

	mono::mono_type m_type;

	std::map<size_t, mono::mono_object>::iterator m_objectIter;


public:
	T& obj() { return m_ref.obj; }
	void obj(const T& value) { m_ref.obj = value; }

	const std::string& nameSpace() { return m_namespace; }
	const std::string& name() { return m_name; }
		
	size_t GetRef() { return m_ref.RefId(); }
	const mono::mono_type& GetType() { return m_type; }

	CSLinked(MonoInst* imono) {
		m_mono = imono;
	}

	void Link(const std::string& name_space, const std::string& name, size_t classInfoRef) {

		m_namespace = name_space;
		m_name = name;

		m_type = m_mono->GetType(m_namespace, m_name);

		auto method_Link = m_type.get_method("Link", 2);
		auto invoker_Link = mono::make_method_invoker<void(size_t, size_t)>(method_Link);

		auto pair = m_mono->m_objects.emplace(std::make_pair(GetRef(), m_type.new_instance()));
		m_objectIter = pair.first;

		auto csObj = (*m_objectIter).second;
		invoker_Link(csObj, classInfoRef, GetRef());
	}
};
