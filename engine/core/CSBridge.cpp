#include "CSBridge.h"

#include "Refs.h"
#include "CppClass.h"
#include "ClassInfo.h"


void CSBridge::setValue_2() {
    std::cout << "+: CSBridge::setValue()" << std::endl;
}

void CSBridge::setValue(CppRef refId, float value) {
    auto* a = CppRefs::GetPointer<CppClass>(refId);
    if (a != nullptr)
        a->floatValue = value;
}

void* CSBridge::Ref_GetPointer(CppRef refId) {
    return CppRefs::GetPointer<void>(refId);
}

void CSBridge::CppClass_GetClassInfo(CppRef refId, ClassInfo* outInfo) {
    std::cout << "+: CSBridge::CppClass_GetClassInfo()" << std::endl;

    auto* info = CppRefs::GetPointer<ClassInfo>(refId);

    if (info != nullptr) {
        outInfo->offsetCount = info->offsetCount;
        outInfo->offsets = info->offsets;
    }
}