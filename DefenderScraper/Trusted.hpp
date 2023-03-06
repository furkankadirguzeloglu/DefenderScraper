#pragma once
#include <Windows.h>
#include <string>
#include <sddl.h>
#include <iostream>
#include "Util.hpp"

namespace Trusted {
	bool ProcessCreate(std::string commandLine);
	bool IsSystemGroup();
	bool HasAdmin();
}
