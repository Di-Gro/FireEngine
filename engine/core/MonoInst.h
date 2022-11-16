#pragma once
#include <map>

#include "monopp/mono_jit.h"
#include "monopp/mono_domain.h"
#include "monopp/mono_assembly.h"
#include "monopp/mono_type.h"
#include "monopp/mono_method_invoker.h"

class MonoInst {
private:
	mono::mono_domain* m_domain = nullptr;
	mono::mono_assembly* m_assembly = nullptr;

	

public:
	std::map<size_t, mono::mono_object> m_objects;

	mono::mono_assembly* assembly() { return m_assembly; }

	mono::mono_type GetType(const std::string& name_space, const std::string& name) {
		return m_assembly->get_type(name_space, name);
	}

	MonoInst(mono::mono_domain* domain, mono::mono_assembly* assembly) {
		m_domain = domain;
		m_assembly = assembly;
	}

};

