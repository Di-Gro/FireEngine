#pragma once

#include <iostream>

static int GetOffset(void* from, void* to) {
	return ((int8_t*)from) - ((int8_t*)to);
}

class CppClass {
public:
	float floatValue = 1.154f;
	int intValue = 1;

private:
	long m_longValue = 1;

public:
	long longValue() { return m_longValue;  }
	void longValue(long value) { m_longValue = value; }

	float SomeFunc() { return intValue * floatValue; }

	static int OffsetCount() { return 2; }

	static void WriteOffsets(int* offsets) {
		auto* ptr = (CppClass*)4;
		offsets[0] = GetOffset(&ptr->floatValue, ptr);
		offsets[1] = GetOffset(&ptr->intValue, ptr);
	}
};

namespace CSBridge {

	extern "C" __declspec(dllexport) long CppClass_longValue_get(size_t objRef);
	extern "C" __declspec(dllexport) void CppClass_longValue_set(size_t objRef, long value);

	extern "C" __declspec(dllexport) float CppClass_SomeFunc(size_t objRef);

}

// floatValue
// intValue
/// идут в C# как свойства с прямым доступом по смещениям из самого C#

// m_longValue
// long longValue()
// void longValue(long value)
/// идет в C# как свойство с доступом через getter и setter из C++
