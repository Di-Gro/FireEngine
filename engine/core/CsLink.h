#pragma once

#include "CSBridge.h"

class Game;
class GameObject;

class CsLink {
	friend class Game;
	friend class GameObject;

public:
	Ref2 f_ref;
	CppRef f_cppRef;
	CsRef f_csRef;

	CppRef cppRef() { return f_cppRef; }
	CsRef csRef() { return f_csRef; }
};