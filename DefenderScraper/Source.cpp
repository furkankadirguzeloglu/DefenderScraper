#include <Windows.h>
#include <iostream>
#include "DefenderControl.hpp"
#include "Trusted.hpp"
#include "wmic.hpp"
#include "Unlocker.hpp"
#include "Logger.h"

HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

void WriteLogo() {
    SetConsoleTextAttribute(hConsole, 11);
    std::cout << R"(

  ____            __                      _                   ____                                              
 |  _ \    ___   / _|   ___   _ __     __| |   ___   _ __    / ___|    ___   _ __    __ _   _ __     ___   _ __ 
 | | | |  / _ \ | |_   / _ \ | '_ \   / _` |  / _ \ | '__|   \___ \   / __| | '__|  / _` | | '_ \   / _ \ | '__|
 | |_| | |  __/ |  _| |  __/ | | | | | (_| | |  __/ | |       ___) | | (__  | |    | (_| | | |_) | |  __/ | |   
 |____/   \___| |_|    \___| |_| |_|  \__,_|  \___| |_|      |____/   \___| |_|     \__,_| | .__/   \___| |_|   
                                                                                           |_|                  
                                                                                                                                                                                                                  
 [+] Developed by Majdev
------------------------------------------------------------------------------------------                                                
)";
}

bool CheckSilent(int argc, char** argv) {
    for (int i = 0; i < argc; i++) {
        if (!strcmp(argv[i], "-s"))
            return true;
    }
    return false;
}

int main(int argc, char** argv) {
    WriteLogo();
    auto Silent = CheckSilent(argc, argv);
    if (!Trusted::HasAdmin()) {
        Logger::Write("Running the program as an administrator!", Logger::LoggerType.Error);

        if (!Silent)
            system("pause");

        return EXIT_FAILURE;
    }

    if (!Trusted::IsSystemGroup()) {
        Logger::Write("Restarting with privileges...", Logger::LoggerType.Warning);
        Trusted::ProcessCreate(Util::GetCurrentPath().append(Silent ? " -s" : ""));
        return EXIT_SUCCESS;
    }

    try {
        Logger::Write("Disabling Windows Defender...", Logger::LoggerType.Info);
        DefenderControl::KillSmartScreen();
        DefenderControl::ManageWinDefend(false);
        DefenderControl::ToggleTamper(false);
        DefenderControl::DisableDefender();
        DefenderControl::DisableDefender();
        DefenderControl::ManageSecurityCenter(false);
        Sleep(3000);
        Logger::Write("Removing Windows Defender Services and Files...", Logger::LoggerType.Info);
        Unlocker::Delete("C:\\Program Files (x86)\\Windows Defender");
        Sleep(3000);
        Unlocker::Delete("C:\\Program Files\\Windows Defender Advanced Threat Protection");
        Sleep(3000);
        Unlocker::Delete("C:\\Program Files\\Windows Defender");
        Sleep(3000);
        Unlocker::Delete("C:\\ProgramData\\Windows Defender");
        Sleep(3000);
        Unlocker::Delete("C:\\ProgramData\\Microsoft\\Windows Defender");
        Sleep(3000);
        Unlocker::Delete("C:\\ProgramData\\Microsoft\\Windows Defender Advanced Threat Protection");
        Sleep(3000);
        Unlocker::Delete("C:\\ProgramData\\Microsoft\\Windows Security Health");
        Sleep(3000);
        Logger::Write("Windows Defender is Permanently Disabled and Deleted!", Logger::LoggerType.Success);
        Logger::Write("System rebooting...", Logger::LoggerType.Info);

        TCHAR Buffer[MAX_PATH];
        GetTempPath(MAX_PATH, Buffer);

        std::string FilePath = Buffer;
        FilePath += "\IObitUnlockerPortable.exe";
        remove(FilePath.c_str());

        FilePath = Buffer;
        FilePath += "\IObitUnlockerPortable";
        remove(FilePath.c_str());

        system("C:\\Windows\\System32\\shutdown /r /t 5");
        Sleep(3000);
        exit(EXIT_SUCCESS);
    }
    catch (std::exception e) {}
    return EXIT_SUCCESS;
}
