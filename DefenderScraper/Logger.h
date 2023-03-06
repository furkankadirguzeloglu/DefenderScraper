#pragma once
#include <iostream>
#include <string>
#include <windows.h>
#include <fstream>

namespace Logger {
    HANDLE  Console = GetStdHandle(STD_OUTPUT_HANDLE);

    struct _LogType {
        int Default = 15;
        int Warning = 6;
        int Success = 2;
        int Error = 4;
        int Info = 11;
    }LoggerType;

    std::string LogFileName = "DefenderScraper.log";
    bool First = true;

    void Log(std::string Text, int LogType = LoggerType.Info) {
        SYSTEMTIME lt;
        GetLocalTime(&lt);
        std::ofstream LogFile;
        LogFile.open(LogFileName, std::ios::app);
        char Time[64];

        if (LogType == LoggerType.Default) {
            sprintf(Time, "[Info | %02d/%02d/%02d %02d:%02d:%02d]: ", lt.wMonth, lt.wDay, lt.wYear, lt.wHour, lt.wMinute, lt.wSecond);
        }

        if (LogType == LoggerType.Info) {
            sprintf(Time, "[Info | %02d/%02d/%02d %02d:%02d:%02d]: ", lt.wMonth, lt.wDay, lt.wYear, lt.wHour, lt.wMinute, lt.wSecond);
        }

        if (LogType == LoggerType.Success) {
            sprintf(Time, "[Success | %02d/%02d/%02d %02d:%02d:%02d]: ", lt.wMonth, lt.wDay, lt.wYear, lt.wHour, lt.wMinute, lt.wSecond);
        }

        if (LogType == LoggerType.Warning) {
            sprintf(Time, "[Warning | %02d/%02d/%02d %02d:%02d:%02d]: ", lt.wMonth, lt.wDay, lt.wYear, lt.wHour, lt.wMinute, lt.wSecond);
        }

        if (LogType == LoggerType.Error) {
            sprintf(Time, "[Error | %02d/%02d/%02d %02d:%02d:%02d]: ", lt.wMonth, lt.wDay, lt.wYear, lt.wHour, lt.wMinute, lt.wSecond);
        }

        if (First == true) {
            LogFile << R"(

  __  __               _   _   _   ___       _                           
 |  \/  |   __ _    __| | | | | | |_ _|     | |       ___     __ _   ___ 
 | |\/| |  / _` |  / _` | | | | |  | |      | |      / _ \   / _` | / __|
 | |  | | | (_| | | (_| | | |_| |  | |      | |___  | (_) | | (_| | \__ \
 |_|  |_|  \__,_|  \__,_|  \___/  |___|     |_____|  \___/   \__, | |___/
                                                             |___/                                                                                                                                                                                          
------------------------------------------------------------------------------------------                                                
)";
            First = false;
        }

        LogFile << Time << Text << std::endl;
        LogFile.close();
    }

    void Write(std::string Message, int LogType = LoggerType.Default, bool WriteLog = false) {
        SYSTEMTIME lt;
        GetLocalTime(&lt);
        SetConsoleTextAttribute(Console, 8);
        printf("[%02d/%02d/%02d %02d:%02d:%02d] ", lt.wMonth, lt.wDay, lt.wYear, lt.wHour, lt.wMinute, lt.wSecond);
        SetConsoleTextAttribute(Console, LogType);
        std::cout << Message << std::endl;
        SetConsoleTextAttribute(Console, 15);
        if (WriteLog == true) {
            Log(Message, LogType);
        }
    }
}