#include "mono_game_types.h"

const CppRef CppRef::Void = RefCpp(0);
const CsRef CsRef::Void = RefCs(0);


std::ostream& operator<< (std::ostream& out, const CsRef& ref) {
	out << "cs:" << ref.value;
	return out;
}

std::ostream& operator<< (std::ostream& out, const CppRef& ref) {
	out << "cpp:" << ref.value;
	return out;
}

CppRef RefCpp(size_t v) {
	CppRef ref;
	ref.value = v;
	return ref;
}

CsRef RefCs(size_t v) {
	CsRef ref;
	ref.value = v;
	return ref;
}