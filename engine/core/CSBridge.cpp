#include "CSBridge.h"

#include "Ref.h"
#include "CppClass.h"
#include "ClassInfo.h"


void CSBridge::setValue_2() {
    std::cout << "CPP: CSBridge::setValue()" << std::endl;
}

void CSBridge::setValue(size_t refId, float value) {
    auto* a = Refs::GetRef<CppClass>(refId);
    if (a != nullptr)
        a->floatValue = value;
}

void* CSBridge::Ref_GetPointer(size_t refId) {
    return Refs::GetRef<void>(refId);
}

void CSBridge::CppClass_GetClassInfo(size_t refId, ClassInfo* outInfo) {
    std::cout << "CPP: CSBridge::CppClass_GetClassInfo()" << std::endl;

    auto* info = Refs::GetRef<ClassInfo>(refId);

    if (info != nullptr) {
        outInfo->offsetCount = info->offsetCount;
        outInfo->offsets = info->offsets;
    }
}