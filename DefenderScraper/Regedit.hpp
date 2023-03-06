#pragma once
#include <Windows.h>
#include <cstdint>
#include <iostream>

namespace Regedit
{
  DWORD ReadKey(const char* RootName, const char* Value, uint32_t Flags);
  bool CreateRegeditistry(const char* RootName, HKEY& hkey);
  bool SetKeyval(HKEY& hkey, const char* Value, DWORD value);
  bool SetKeyvalBinary(HKEY& hkey, const char* Value, DWORD value);
}
