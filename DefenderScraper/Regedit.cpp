#include "Regedit.hpp"

namespace Regedit {
    DWORD ReadKey(const char* RootName, const char* Value, uint32_t Flags) {
        LSTATUS Status;
        HKEY hKey;
        DWORD Result{};
        DWORD buff_sz = sizeof(DWORD);
        Status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, RootName, 0, KEY_READ | KEY_WOW64_64KEY, &hKey);
        if (Status) {
            return -1;
        }

        Status = RegQueryValueEx(hKey, Value, 0, NULL, reinterpret_cast<LPBYTE>(&Result), &buff_sz);
        if (Status) {
            return -1;
        }

        RegCloseKey(hKey);
        return Result;
    }

    bool CreateRegeditistry(const char* RootName, HKEY& hkey) {
        LSTATUS Status;
        DWORD Disposition;

        Status = RegCreateKeyEx(HKEY_LOCAL_MACHINE, RootName, 0, 0, 0, 131334, 0, &hkey, &Disposition);
        if (Status) {
            return false;
        }
        return true;
    }

    bool SetKeyval(HKEY& hkey, const char* Value, DWORD value) {
        auto Result = RegSetValueEx(hkey, Value, 0, REG_DWORD, reinterpret_cast<LPBYTE>(&value), 4);
        if (Result) {
            return false;
        }
        return true;
    }

    bool SetKeyvalBinary(HKEY& hkey, const char* Value, DWORD value) {
        auto Result = RegSetValueEx(hkey, Value, 0, REG_BINARY, reinterpret_cast<LPBYTE>(&value), 12);
        if (Result) {
            return false;
        }
        return true;
    }
}
