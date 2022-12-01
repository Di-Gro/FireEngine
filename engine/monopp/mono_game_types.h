#pragma once

#include <iostream>


class CppRef {
public:
	static const CppRef Void;

	size_t value = 0;

	//CppRef() {}
	//CppRef(size_t v) : value(v) {}

	operator size_t() const { return value; }

	CppRef& operator=(size_t v) {
		value = v; 
		return *this;
	}

	static CppRef Create(size_t v) {
		CppRef ref;
		ref.value = v;
		return ref;
	}

	friend std::ostream& operator<< (std::ostream& out, const CppRef& ref);
};

class CsRef {
public:
	static const CsRef Void;

	size_t value = 0;

	//CsRef() {}
	//CsRef(size_t v) : value(v) {}

	operator size_t() const { return value; }

	CsRef& operator=(size_t v) {
		value = v;
		return *this;
	}

	static CsRef Create(size_t v) {
		CsRef ref;
		ref.value = v;
		return ref;
	}

	friend std::ostream& operator<< (std::ostream& out, const CsRef& ref);
};

class CppObjectInfo {
public:
	CppRef cppRef;
	CppRef classRef;
};

class GameObjectInfo {
public:
	CppRef objectRef;
	CppRef classRef;
	CsRef transformRef;
};

class GameUpdateData {
public:
	float deltaTime;
};

CppRef RefCpp(size_t v);
CsRef RefCs(size_t v);