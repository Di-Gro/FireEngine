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
