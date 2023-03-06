#include "Unlocker.hpp"
#include "IObitUnlockerPortable.h"

namespace Unlocker
{
	bool FileExists(std::string Path) {
		struct stat Buffer;
		return (stat(Path.c_str(), &Buffer) == 0);
	}

	void Delete(std::string Path) {
		TCHAR Buffer[MAX_PATH];
		GetTempPath(MAX_PATH, Buffer);

		std::string FilePath = Buffer;
		FilePath += "\IObitUnlockerPortable.exe";

		if (FileExists(FilePath) == false) {
			Util::WriteFile(DataIObitUnlockerPortable, sizeof(DataIObitUnlockerPortable), FilePath);
		}
		
		Path = "-delete \"" + Path + "\"";
		Util::ProcessStart(FilePath, Path);
	}
}