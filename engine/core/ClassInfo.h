#pragma once
#include <iostream>


class ClassInfo {
public:
	int offsetCount = 0;
	int* offsets = nullptr;

	ClassInfo() {}

	ClassInfo(size_t count, void(*writeOffsets)(int*)) {

		offsetCount = count;

		offsets = new int[offsetCount];
		writeOffsets(offsets);
	}

	ClassInfo(ClassInfo&& other) noexcept {
		this->operator=(std::move(other));
	}

	ClassInfo& operator=(ClassInfo&& other) noexcept {
		if (this != &other) {
			offsetCount = other.offsetCount;
			offsets = other.offsets;
			other.offsets = nullptr;
		}
		return *this;
	}

	~ClassInfo() {
		if (offsets != nullptr)
			delete[] offsets;
	}

	template<typename TClass>
	static ClassInfo Create() {
		auto info = ClassInfo(TClass::OffsetCount(), [](int* offsets) {TClass::WriteOffsets(offsets); });
		return info;
	}

	void Print() {
		std::cout << "ClassInfo (" << offsetCount << ", " << (size_t)offsets << ")" << std::endl;
		for (int i = 0; i < offsetCount; i++)
			std::cout << offsets[i] << std::endl;
	}


};



