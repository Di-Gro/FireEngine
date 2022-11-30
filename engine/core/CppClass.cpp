#include "CppClass.h"

#include "Refs.h"
#include "CSBridge.h"

DEF_OBJECT(CppClass, 4) {
	offsets[0] = offsetof(CppClass, floatValue);
	offsets[1] = offsetof(CppClass, intValue);
	offsets[2] = offsetof(CppClass, vector3);
	offsets[3] = offsetof(CppClass, quat);
}

DEF_PROP_GETSET(CppClass, long, longValue)
DEF_PROP_GETSET(CppClass, Vector3, vector3m)

DEF_FUNC(CppClass, SomeFunc, float) (CppRef objRef) {
	auto* a = CppRefs::GetPointer<CppClass>(objRef);
	if (a != nullptr)
		return a->SomeFunc();
	return 0;
}