#pragma once
#include <Windows.h>
#include <string>
#include <lmcons.h>
#include <TlHelp32.h>

namespace Util {
	std::wstring StringToWString(const std::string& s);
	std::string GetCurrentPath();
	DWORD GetProcessID(std::string process_name);
	void ProcessStart(std::string Path, std::string CommandLine);
	void WriteFile(unsigned char* Buffer, int Size, std::string Path);
}
