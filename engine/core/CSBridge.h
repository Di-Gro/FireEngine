#pragma once

#include <iostream>

#include "Refs.h"
//#include "CS.h"

class ClassInfo;

template<typename T>
concept HasMetaOffsets =
requires(T a, int* b) {
	{ T::meta_offsets };
	{ T::meta_offsetCount };
	{ T::meta_WriteOffsets(b)} -> std::same_as<void>;
};

template<typename T>
concept IsCppComponent =
requires(T a, int* b) {
	{ T::meta_csComponentName };
	{ T::meta_offsets };
	{ T::meta_offsetCount };
	{ T::meta_WriteOffsets(b)} -> std::same_as<void>;
};

struct CppObjectInfo {
	CppRef cppRef;
	CppRef classRef;
};

struct GameObjectInfo {
	CppRef objectRef;
	CppRef classRef;
	CsRef transformRef;
};

namespace CSBridge {

    extern "C" __declspec(dllexport) void setValue_2();
    extern "C" __declspec(dllexport) void setValue(CppRef refId, float value);

    extern "C" __declspec(dllexport) void CppClass_GetClassInfo(CppRef refId, ClassInfo * outInfo);

    extern "C" __declspec(dllexport) void* Ref_GetPointer(CppRef objectRefId);

}

#define PROP_GET(ClassName, propType, propName) extern "C" __declspec(dllexport) propType ClassName##_##propName##_get(CppRef objRef);\

#define PROP_SET(ClassName, propType, propName) extern "C" __declspec(dllexport) void ClassName##_##propName##_set(CppRef objRef, propType value);\

#define PROP_GETSET(ClassName, propType, propName)\
	PROP_GET(ClassName, propType, propName)\
	PROP_SET(ClassName, propType, propName)\

#define DEF_PROP_GET(ClassName, propType, propName)\
	propType ClassName##_##propName##_get(CppRef objRef) {\
		auto* a = Refs::GetPointer<ClassName>(objRef);\
		if (a != nullptr)\
			return a->propName();\
		else\
			std::cout << "+: prop_get::GetPointer<" << #ClassName << ">(" << objRef << "): NULL" << std::endl;\
\
		return propType();\
	}\

#define DEF_PROP_SET(ClassName, propType, propName)\
	void ClassName##_##propName##_set(CppRef objRef, propType value) {\
		auto* a = Refs::GetPointer<ClassName>(objRef);\
		if (a != nullptr)\
			a->propName(value);\
		else\
			std::cout << "+: prop_set::GetPointer<" << #ClassName << ">(" << objRef << "): NULL" << std::endl;\
\
	}\

#define DEF_PROP_GETSET(ClassName, propType, propName)\
	DEF_PROP_GET(ClassName, propType, propName)\
	DEF_PROP_SET(ClassName, propType, propName)\

#define FUNC(ClassName, funcName, retType) extern "C" __declspec(dllexport) retType ClassName##_##funcName

#define DEF_FUNC(ClassName, funcName, retType) retType ClassName##_##funcName 

#define DEC_COMPONENT_CREATE(ClassName)\
FUNC(ClassName, Create, CppObjectInfo)(CppRef cppObjRef, CsRef csCompRef) \

#define DEF_COMPONENT_CREATE(ClassName)\
DEF_FUNC(ClassName, Create, CppObjectInfo)(CppRef cppObjRef, CsRef csCompRef) {\
	CppObjectInfo strct;\
\
	auto* gameObject = Refs::GetPointer<GameObject>(cppObjRef);\
	if (gameObject != nullptr) {\
		auto classInfoRef = Refs::Create(ClassInfo::Get<ClassName>());\
\
		strct.cppRef = gameObject->CreateComponent<ClassName>(csCompRef);\
		strct.classRef = classInfoRef.id();\
\
		return strct;\
	}\
	return strct;\
}\

#define COMPONENT public:\
static ClassInfo meta_offsets;\
static const char* meta_csComponentName;\
static const int meta_offsetCount;\
static void meta_WriteOffsets(int* offsets) \

#define DEC_COMPONENT(ClassName)\
DEC_COMPONENT_CREATE(ClassName)\

#define DEF_COMPONENT(ClassName, CsClassName, offsetsCount)\
ClassInfo ClassName##::meta_offsets;\
const int ClassName##::meta_offsetCount = offsetsCount;\
const char* ClassName##::meta_csComponentName = #CsClassName;\
DEF_COMPONENT_CREATE(ClassName)\
void ClassName##::meta_WriteOffsets(int* offsets)\


#define OBJECT public:\
static ClassInfo meta_offsets;\
static const int meta_offsetCount;\
static void meta_WriteOffsets(int* offsets) \

#define DEF_OBJECT(ClassName, offsetsCount)\
ClassInfo ClassName##::meta_offsets;\
const int ClassName##::meta_offsetCount = offsetsCount;\
void ClassName##::meta_WriteOffsets(int* offsets)\

#define OFFSET(index, ClassName, fieldName)\
offsets[index] = offsetof(ClassName, fieldName)\
