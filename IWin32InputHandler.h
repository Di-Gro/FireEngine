#pragma once

#include <windows.h>


class IWin32InputHandler {

public:
	virtual void OnInput(LPARAM lparam) {}

};