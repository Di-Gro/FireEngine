#pragma once

#include <iostream>

#include "Refs.h"
#include <mono_game_types.h>
#include "RunMode.h"

class ClassInfo;

namespace CSBridge {

    extern "C" __declspec(dllexport) void setValue_2();
    extern "C" __declspec(dllexport) void setValue(CppRef refId, float value);

    extern "C" __declspec(dllexport) void CppClass_GetClassInfo(CppRef refId, ClassInfo * outInfo);

    extern "C" __declspec(dllexport) void* Ref_GetPointer(CppRef objectRefId);

}

#define PROP_GET(ClassName, propType, propName) extern "C" __declspec(dllexport) propType ClassName##_##propName##_get(CppRef objRef);\

#define PROP_SET(ClassName, propType, propName) extern "C" __declspec(dllexport) void ClassName##_##propName##_set(CppRef objRef, propType value);\

#define PROP_GET_STR(ClassName, propName) extern "C" __declspec(dllexport) size_t ClassName##_##propName##_get(CppRef objRef);\

#define PROP_SET_STR(ClassName, propName) extern "C" __declspec(dllexport) void ClassName##_##propName##_set(CppRef objRef, const char* value);\

#define PROP_GET_CSTR(ClassName, propName) extern "C" __declspec(dllexport) size_t ClassName##_##propName##_get(CppRef objRef);\


#define PROP_GETSET(ClassName, propType, propName)\
	PROP_GET(ClassName, propType, propName)\
	PROP_SET(ClassName, propType, propName)\

#define PROP_GETSET_E(ClassName, propType, propName)\
	PROP_GET(ClassName, int, propName)\
	PROP_SET(ClassName, int, propName)\

#define PROP_GETSET_STR(ClassName, propName)\
	PROP_GET_STR(ClassName, propName)\
	PROP_SET_STR(ClassName, propName)\

#define PROP_GETSET_CSTR(ClassName, propName)\
	PROP_GET_CSTR(ClassName, propName)\
	PROP_SET(ClassName, const char*, propName)\

#define DEF_PROP_GETSET(ClassName, propType, propName)\
	DEF_PROP_GET(ClassName, propType, propName)\
	DEF_PROP_SET(ClassName, propType, propName)\

#define DEF_PROP_GETSET_E(ClassName, propType, propName)\
	DEF_PROP_GET_E(ClassName, propType, propName)\
	DEF_PROP_SET_E(ClassName, propType, propName)\

#define DEF_PROP_GETSET_F(ClassName, propType, propName, field)\
	DEF_PROP_GET_F(ClassName, propType, propName, field)\
	DEF_PROP_SET_F(ClassName, propType, propName, field)\

#define DEF_PROP_GETSET_STR(ClassName, propName)\
	DEF_PROP_GET_STR(ClassName, propName)\
	DEF_PROP_SET_STR(ClassName, propName)\

#define DEF_PROP_GETSET_CSTR(ClassName, propName)\
	DEF_PROP_GET_CSTR(ClassName, propName)\
	DEF_PROP_SET(ClassName, const char*, propName)\


#define DEF_PROP_GET(ClassName, propType, propName)\
	propType ClassName##_##propName##_get(CppRef objRef) {\
		auto* a = CppRefs::GetPointer<ClassName>(objRef);\
		if (a != nullptr)\
			return a->propName();\
		else\
			std::cout << "+: prop_get::GetPointer<" << #ClassName << ">(" << objRef << "): NULL" << std::endl;\
\
		return propType();\
	}\

#define DEF_PROP_SET(ClassName, propType, propName)\
	void ClassName##_##propName##_set(CppRef objRef, propType value) {\
		auto* a = CppRefs::GetPointer<ClassName>(objRef);\
		if (a != nullptr)\
			a->propName(value);\
		else\
			std::cout << "+: prop_set::GetPointer<" << #ClassName << ">(" << objRef << "): NULL" << std::endl;\
\
	}\

#define DEF_PROP_GET_E(ClassName, propType, propName)\
	int ClassName##_##propName##_get(CppRef objRef) {\
		auto* a = CppRefs::GetPointer<ClassName>(objRef);\
		if (a != nullptr)\
			/* TODO: Почему здесь преобразование к int, а в DEF_PROP_SET_E к propType? */\
			return (int)a->propName();\
		else\
			std::cout << "+: prop_get::GetPointer<" << #ClassName << ">(" << objRef << "): NULL" << std::endl;\
\
		return 0;\
	}\

#define DEF_PROP_SET_E(ClassName, propType, propName)\
	void ClassName##_##propName##_set(CppRef objRef, int value) {\
		auto* a = CppRefs::GetPointer<ClassName>(objRef);\
		if (a != nullptr)\
			a->propName((propType)value);\
		else\
			std::cout << "+: prop_set::GetPointer<" << #ClassName << ">(" << objRef << "): NULL" << std::endl;\
\
	}\

#define DEF_PROP_GET_F(ClassName, propType, propName, field)\
	propType ClassName##_##propName##_get(CppRef objRef) {\
		auto* a = CppRefs::GetPointer<ClassName>(objRef);\
		if (a != nullptr)\
			return a->field;\
		else\
			std::cout << "+: prop_get::GetPointer<" << #ClassName << ">(" << objRef << "): NULL" << std::endl;\
\
		return propType();\
	}\

#define DEF_PROP_SET_F(ClassName, propType, propName, field)\
	void ClassName##_##propName##_set(CppRef objRef, propType value) {\
		auto* a = CppRefs::GetPointer<ClassName>(objRef);\
		if (a != nullptr)\
			a->field = value;\
		else\
			std::cout << "+: prop_set::GetPointer<" << #ClassName << ">(" << objRef << "): NULL" << std::endl;\
\
	}\

#define DEF_PROP_GET_STR(ClassName, propName)\
	size_t ClassName##_##propName##_get(CppRef objRef) {\
		auto* obj = CppRefs::ThrowPointer<ClassName>(objRef);\
		return (size_t)obj->propName().c_str();\
	}\

#define DEF_PROP_SET_STR(ClassName, propName)\
	void ClassName##_##propName##_set(CppRef objRef, const char* value) {\
		auto* obj = CppRefs::ThrowPointer<ClassName>(objRef);\
		obj->propName(value);\
	}\

#define DEF_PROP_GET_CSTR(ClassName, propName)\
	size_t ClassName##_##propName##_get(CppRef objRef) {\
		auto* a = CppRefs::GetPointer<ClassName>(objRef);\
		if (a != nullptr)\
			return (size_t)a->propName();\
		else\
			std::cout << "+: prop_get::GetPointer<" << #ClassName << ">(" << objRef << "): NULL" << std::endl;\
		throw std::exception();\
	}\


#define FUNC(ClassName, funcName, retType) extern "C" __declspec(dllexport) retType ClassName##_##funcName

#define FRIEND_FUNC(ClassName, funcName, retType) friend retType ClassName##_##funcName

#define DEF_FUNC(ClassName, funcName, retType) retType ClassName##_##funcName 

#define PUSH_ASSET(ClassName)\
FUNC(ClassName, PushAsset, CppRef)(CppRef gameRef, const char* assetId, int assetIdHash);\
PROP_GETSET_CSTR(ClassName, assetId)\
PROP_GETSET(ClassName, int, assetIdHash)\

#define DEF_PUSH_ASSET(ClassName)\
DEF_FUNC(ClassName, PushAsset, CppRef)(CppRef gameRef, const char* assetId, int assetIdHash) {\
	auto game = CppRefs::ThrowPointer<Game>(gameRef);\
\
	auto* asset = (ClassName*)game->assets()->GetStatic(assetIdHash);\
	if (asset == nullptr) {\
		asset = new ClassName();\
		asset->assetId(assetId);\
		asset->assetIdHash(assetIdHash);\
		game->assets()->Push(assetId, asset);\
	}\
	return CppRefs::GetRef(asset);\
}\
DEF_PROP_GETSET_CSTR(ClassName, assetId)\
DEF_PROP_GETSET(ClassName, int, assetIdHash)\


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



#define DEC_COMPONENT_CREATE(ClassName)\
FUNC(ClassName, Create, CppObjectInfo)(CsRef csCompRef) \

#define DEF_COMPONENT_CREATE(ClassName)\
DEF_FUNC(ClassName, Create, CppObjectInfo)(CsRef csCompRef) {\
\
	Component* component = Actor::inner_CreateComponent<ClassName>(csCompRef);\
	auto meta = component->GetMeta();\
\
	CppObjectInfo strct;\
	strct.cppRef = component->cppRef();\
	strct.classRef = meta.classInfoRef;\
\
	return strct;\
}\

#define DEF_COMPONENT_WRITE_OFFSETS_METHOD(ClassName)\
void ClassName##::meta_WriteOffsets(int* offsets)\

#define COMPONENT_STATIC_MEMBERS(ClassName)\
private:\
	static Ref2 meta_classInfoRef;\
	static ClassInfo meta_classInfo;\
	static const char* meta_csComponentName;\
	static const int meta_offsetCount;\
	static const RunMode meta_RunMode;\
	static void meta_WriteOffsets(int* offsets); \

#define DEF_COMPONENT_STATIC_MEMBERS(ClassName, CsClassName, offsetsCount, runMode)\
Ref2 ClassName##::meta_classInfoRef;\
ClassInfo ClassName##::meta_classInfo;\
const char* ClassName##::meta_csComponentName = #CsClassName;\
const int ClassName##::meta_offsetCount = offsetsCount;\
const RunMode ClassName##::meta_RunMode = runMode;\

#define COMPONENT_VIRTUAL_MEHTODS(ClassName, isPureComponent)\
public:\
ComponentMeta GetMeta() override {\
	if (!meta_classInfo.IsInited()) {\
		meta_classInfo = ClassInfo(meta_offsetCount, [](int* offsets) {meta_WriteOffsets(offsets); });\
		meta_classInfoRef = CppRefs::Create(&meta_classInfo);\
	}\
	ComponentMeta meta;\
	meta.isPure = isPureComponent;\
	meta.name = meta_csComponentName;\
	meta.classInfo = &meta_classInfo;\
	meta.classInfoRef = meta_classInfoRef.cppRef();\
	return meta;\
}\
bool NeedRunInEditor() override {\
	return meta_RunMode != RunMode::PlayOnly;\
}\
bool NeedRunInPlayer() override {\
	return meta_RunMode != RunMode::EditOnly;\
}\
private:\

#define COMPONENT_CONSTRUCTOR(ClassName)\
protected:\
	ClassName##() {}\
private:\



/// COMPONENT
/// ->

#define COMPONENT(ClassName)\
COMPONENT_STATIC_MEMBERS(ClassName)\
COMPONENT_VIRTUAL_MEHTODS(ClassName, false)\

#define DEC_COMPONENT(ClassName)\
DEC_COMPONENT_CREATE(ClassName)\

#define DEF_COMPONENT(ClassName, CsClassName, offsetsCount, playMode)\
DEF_COMPONENT_STATIC_MEMBERS(ClassName, CsClassName, offsetsCount, playMode)\
DEF_COMPONENT_CREATE(ClassName)\
DEF_COMPONENT_WRITE_OFFSETS_METHOD(ClassName)\

/// <-
/// COMPONENT

/// PURE_COMPONENT
/// ->

#define PURE_COMPONENT(ClassName)\
COMPONENT_STATIC_MEMBERS(ClassName)\
COMPONENT_VIRTUAL_MEHTODS(ClassName, true)\

#define DEF_PURE_COMPONENT(ClassName, playMode)\
DEF_COMPONENT_STATIC_MEMBERS(ClassName, ClassName, 0, playMode)\
DEF_COMPONENT_WRITE_OFFSETS_METHOD(ClassName){ }\

/// <-
/// PURE_COMPONENT