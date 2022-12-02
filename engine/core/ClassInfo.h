#pragma once
#include <iostream>

#include "CSBridge.h"
#include "ActorConcepts.h"

class ClassInfo {
public:
	int offsetCount = 0;
	int* offsets = nullptr;

private:
	bool m_inited = false;

public:
	bool IsInited() { return m_inited; }

	ClassInfo() {}

	ClassInfo(size_t count, void(*writeOffsets)(int*)) {

		offsetCount = count;

		if (offsetCount > 0) {
			offsets = new int[offsetCount];
			writeOffsets(offsets);
		}
		m_inited = true;
	}

	ClassInfo(ClassInfo&& other) noexcept {
		this->operator=(std::move(other));
		m_inited = true;
	}

	ClassInfo& operator=(ClassInfo&& other) noexcept {
		if (this != &other) {
			offsetCount = other.offsetCount;
			offsets = other.offsets;
			m_inited = true;
			other.offsets = nullptr;
			other.m_inited = false;
		}
		return *this;
	}

	~ClassInfo() {
		if (offsets != nullptr)
			delete[] offsets;
	}

	template<HasMetaOffsets TClass>
	static ClassInfo Create() {
		auto info = ClassInfo(TClass::meta_offsetCount, [](int* offsets) {TClass::meta_WriteOffsets(offsets); });
		return info;
	}

	template<HasMetaOffsets TClass>
	static ClassInfo* Get() {
		ClassInfo& info = TClass::meta_offsets;

		if (!info.m_inited)
			TClass::meta_offsets = ClassInfo::Create<TClass>();

		return &TClass::meta_offsets;
	}

	void Print() {
		std::cout << "ClassInfo (" << offsetCount << ", " << (size_t)offsets << ")" << std::endl;
		for (int i = 0; i < offsetCount; i++)
			std::cout << offsets[i] << std::endl;
	}


};



