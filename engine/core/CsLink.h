#pragma once

#include "CSBridge.h"

class Game;
class Actor;

class CsLink {
	friend class Game;
	friend class Actor;

public:
	Ref2 f_ref;
	CppRef f_cppRef;
	CsRef f_csRef;

	CppRef cppRef() const { return f_cppRef; }
	CsRef csRef() const { return f_csRef; }
};