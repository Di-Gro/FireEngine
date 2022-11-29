#pragma once
#include <map>

#include "Refs.h"
#include "MonoInst.h"
#include "ClassInfo.h"

#include "monopp/mono_jit.h"
#include "monopp/mono_domain.h"
#include "monopp/mono_assembly.h"
#include "monopp/mono_type.h"
#include "monopp/mono_method_invoker.h"

//class CSMethodBase {
//public:
//	template<typename... Args>
//	void Invoke(Args... args) {
//		return m_method_invoker.invoke(m_obj, std::forward<Args>(args)...);
//	}
//};

//template<typename RetType, typename... Args>
//class CSMethod {
//private:
//	const mono::mono_object* m_obj;
//	mono::mono_method m_method;
//	mono::mono_method_invoker<RetType(Args...)> m_method_invoker;
//
//public:
//	CSMethod(const mono::mono_object& obj, const std::string& name, int argc)
//		: m_obj(&obj), m_method(name, argc), m_method_invoker(m_method)
//	{
//
//	}
//	
//	RetType operator()(Args... args) {
//		return m_method_invoker.invoke(m_obj, std::forward<Args>(args)...);
//	}
//};


template<HasMetaOffsets T>
class CSLinked {
private:
	Ref2 m_ref;
public:
	size_t m_csRef;
private:

	MonoInst* m_mono = nullptr;

	std::string m_namespace;
	std::string m_name;

	mono::mono_type m_type;

	std::map<size_t, mono::mono_object>::iterator m_objectIter;


public:
	//T* obj() { return Refs::GetPointer(m_ref); }
	//void obj(const T& value) { m_ref.obj = value; }

	size_t csRef() { return m_csRef; }
	size_t cppRef() { return m_ref.id(); }

	const mono::mono_object& csObj() {return (*m_objectIter).second; }
	

	const std::string& nameSpace() { return m_namespace; }
	const std::string& name() { return m_name; }

	const mono::mono_type& GetType() { return m_type; }

	CSLinked() { }

	CSLinked(MonoInst* imono) { 
		m_mono = imono;
	}

	void PreLink(T& cppObj) {
		m_ref = Refs::Create(&cppObj);
	}

	//void CreateInstance(const std::string& name_space, const std::string& name) {
	//	m_namespace = name_space;
	//	m_name = name;

	//	m_type = m_mono->GetType(m_namespace, m_name);

	//	auto pair = m_mono->m_objects.emplace(std::make_pair( /* TODO */ нет cppRef CppRef(), m_type.new_instance()));
	//	m_objectIter = pair.first;
	//}

	void Link( T& cppObj, const std::string& name_space, const std::string& name) {
		PreLink(cppObj);
		// m_ref =  // Refs::Create<T>(&cppObj);

		m_namespace = name_space;
		m_name = name;

		m_type = m_mono->GetType(m_namespace, m_name);

		auto method_Link = m_type.get_method("Link", 2);
		auto invoker_Link = mono::make_method_invoker<CsRef(CppRef, CppRef)>(method_Link);

		auto pair = m_mono->m_objects.emplace(std::make_pair(cppRef(), m_type.new_instance()));
		m_objectIter = pair.first;

		auto csObj = (*m_objectIter).second;
		auto classInfoRef = Refs::Create(ClassInfo::Get<T>());

		CppRef classRef = RefCpp(classInfoRef.id());
		CppRef objRef = RefCpp(cppRef());

		m_csRef = invoker_Link(csObj, classRef, objRef);
	}

	//template <typename RetType, typename... Args>
	//CSMethod<RetType, Args...> GetMethod(const std::string& name, int argc) {

	//	auto method = CSMethod<RetType, Args...>(csObj(), name, argc);

	//	return method;
	//}
};
