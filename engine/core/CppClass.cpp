#include "CppClass.h"

#include "Ref.h"


namespace CSBridge {

	long CppClass_longValue_get(size_t objRef) {
		auto* a = Refs::GetRef<CppClass>(objRef);
		if (a != nullptr)
			return a->longValue();
		return 0;
	}

	void CppClass_longValue_set(size_t objRef, long value) {
		auto* a = Refs::GetRef<CppClass>(objRef);
		if (a != nullptr)
			a->longValue(value);
	}

	float CppClass_SomeFunc(size_t objRef) {
		auto* a = Refs::GetRef<CppClass>(objRef);
		if (a != nullptr)
			return a->SomeFunc();
		return 0;
	}

}