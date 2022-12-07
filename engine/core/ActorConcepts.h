#pragma once

#include <type_traits>
#include "ComponentMeta.h"


template<typename T>
concept HasMetaOffsets =
requires(T a, int* b) {
	{ T::meta_offsets };
	{ T::meta_offsetCount };
	{ T::meta_WriteOffsets(b)} -> std::same_as<void>;
};

class CsComponent;

template<typename T>
concept IsCsComponent = std::same_as<CsComponent, T>;

template<typename T>
concept HasComponentMeta = 
requires(T a) {
	{ a.GetMeta() } -> std::same_as<ComponentMeta>;
	{ new T() };
};

template<typename T>
concept IsCppAddableComponent = !IsCsComponent<T> && HasComponentMeta<T>;

//template<typename T>
//concept HasCsMetaData =
//requires(T a, int* b) {
//	{ T::meta_csComponentName };
//	{ T::meta_offsets };
//	{ T::meta_offsetCount };
//	{ T::meta_WriteOffsets(b)} -> std::same_as<void>;
//};

//template<typename T>
//concept HasPureName =
//requires(T a) {
//	{ T::meta_csComponentName == "" };
//};
//
//template<typename TComponent, typename TComponentBase>
//concept IsCsCppComponent = std::is_base_of_v<TComponentBase, TComponent> && HasCsMetaData<TComponent>;
//
//template<typename TComponent, typename TComponentBase>
//concept IsPureCppComponent = std::is_base_of_v<TComponentBase, TComponent> && !HasCsMetaData<TComponent>;


//template<typename TComponent>
//concept IsCspp = HasCsMetaData<TComponent>;
//
//template<typename TComponent>
//concept IsPure = HasPureName<TComponent>;

//template<typename T>
//concept IsCsCppComponent = IsComponent<T> && HasCsMetaData<T>;
//
//template<typename T>
//concept IsPureCppComponent = IsComponent<T> && !HasCsMetaData<T>;
//
//template<typename T>
//concept HasMetaOffsets =
//requires(T a, int* b) {
//	{ T::meta_offsets };
//	{ T::meta_offsetCount };
//	{ T::meta_WriteOffsets(b)} -> std::same_as<void>;
//};
//
//template<typename T>
//concept IsComponent = std::is_base_of<Component, T>::value;
//
//template<typename T>
//concept HasCsMetaData =
//requires(T a, int* b) {
//	{ T::meta_csComponentName };
//	{ T::meta_offsets };
//	{ T::meta_offsetCount };
//	{ T::meta_WriteOffsets(b)} -> std::same_as<void>;
//};
//
//template<typename T>
//concept IsCsCppComponent = IsComponent<T> && HasCsMetaData<T>;
//
//template<typename T>
//concept IsPureCppComponent = IsComponent<T> && !HasCsMetaData<T>;