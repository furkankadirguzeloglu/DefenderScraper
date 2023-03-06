#include "Util.hpp"

namespace Util {
    std::wstring StringToWString(const std::string& s) {
        std::wstring temp(s.length(), L' ');
        std::copy(s.begin(), s.end(), temp.begin());
        return temp;
    }

    std::string GetCurrentPath() {
        char buf[256];
        DWORD len = sizeof(buf);
        int bytes = GetModuleFileNameA(NULL, buf, len);
        return std::string(buf);
    }

    DWORD GetProcessID(std::string process_name) {
        HANDLE hSnapshot;
        if ((hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0)) == INVALID_HANDLE_VALUE)
            return -1;

        DWORD pid = -1;
        PROCESSENTRY32 pe;
        ZeroMemory(&pe, sizeof(PROCESSENTRY32));
        pe.dwSize = sizeof(PROCESSENTRY32);

        if (!Process32First(hSnapshot, &pe)) {
            CloseHandle(hSnapshot);
            return -1;
        }

        while (Process32Next(hSnapshot, &pe)) {
            if (pe.szExeFile == process_name) {
                pid = pe.th32ProcessID;
                break;
            }
        }

        if (pid == -1) {
            CloseHandle(hSnapshot);
            return -1;
        }

        CloseHandle(hSnapshot);
        return pid;
    }

    void ProcessStart(std::string Path, std::string CommandLine) {
        STARTUPINFO StartInfo = { sizeof(StartInfo) };
        PROCESS_INFORMATION ProcessInfo;
        if (CreateProcess(Path.c_str(), const_cast<char*>(CommandLine.c_str()), NULL, NULL, TRUE, 0, NULL, NULL, &StartInfo, &ProcessInfo)) {
            WaitForSingleObject(ProcessInfo.hProcess, INFINITE);
        }
    }

    void WriteFile(unsigned char* Buffer, int Size, std::string Path) {
        FILE* File = fopen(Path.c_str(), "wb");
        fwrite(Buffer, sizeof(char), Size, File);
        fclose(File);
    }
}