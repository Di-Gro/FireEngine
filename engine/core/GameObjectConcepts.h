#pragma once

#include <type_traits>


template<typename T>
concept HasMetaOffsets =
requires(T a, int* b) {
	{ T::meta_offsets };
	{ T::meta_offsetCount };
	{ T::meta_WriteOffsets(b)} -> std::same_as<void>;
};

template<typename T>
concept HasCsMetaData =
requires(T a, int* b) {
	{ T::meta_csComponentName };
	{ T::meta_offsets };
	{ T::meta_offsetCount };
	{ T::meta_WriteOffsets(b)} -> std::same_as<void>;
};

template<typename T>
concept HasPureName =
requires(T a) {
	{ T::meta_csComponentName == "" };
};
//
//template<typename TComponent, typename TComponentBase>
//concept IsCsCppComponent = std::is_base_of_v<TComponentBase, TComponent> && HasCsMetaData<TComponent>;
//
//template<typename TComponent, typename TComponentBase>
//concept IsPureCppComponent = std::is_base_of_v<TComponentBase, TComponent> && !HasCsMetaData<TComponent>;


template<typename TComponent>
concept IsCspp = HasCsMetaData<TComponent>;

template<typename TComponent>
concept IsPure = HasPureName<TComponent>;

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