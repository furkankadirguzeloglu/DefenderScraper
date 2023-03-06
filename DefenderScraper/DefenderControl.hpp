#pragma once
#include <Windows.h>
#include <iostream>
#include "Regedit.hpp"
#include "Util.hpp"
#include "Wmic.hpp"

namespace DefenderControl {
	void ToggleTamper(bool enable);
	bool DisableDefender();
	void KillSmartScreen();
	bool ManageWinDefend(bool enable);
	bool ManageSecurityCenter(bool enable);
}