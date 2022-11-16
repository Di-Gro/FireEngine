#pragma once

#include <iostream>

class ClassInfo;

namespace CSBridge {

    extern "C" __declspec(dllexport) void setValue_2();
    extern "C" __declspec(dllexport) void setValue(size_t refId, float value);

    extern "C" __declspec(dllexport) void CppClass_GetClassInfo(size_t refId, ClassInfo * outInfo);

    extern "C" __declspec(dllexport) void* Ref_GetPointer(size_t objectRefId);

}
