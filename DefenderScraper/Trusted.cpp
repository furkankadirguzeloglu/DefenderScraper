#include "Trusted.hpp"

namespace Trusted {
    bool EnablePrivilege(std::string privilege) {
        HANDLE hToken;
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &hToken))
            return false;

        LUID luid;
        if (!LookupPrivilegeValueA(nullptr, privilege.c_str(), &luid)) {
            CloseHandle(hToken);
            return false;
        }

        TOKEN_PRIVILEGES tp;
        tp.PrivilegeCount = 1;
        tp.Privileges[0].Luid = luid;
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), nullptr, nullptr))
        {
            CloseHandle(hToken);
            return false;
        }

        CloseHandle(hToken);
        return true;
    }

    bool ImpersonateSystem() {
        auto systemPid = Util::GetProcessID("winlogon.exe");
        HANDLE hSystemProcess;
        if ((hSystemProcess = OpenProcess(
            PROCESS_DUP_HANDLE | PROCESS_QUERY_INFORMATION,
            FALSE,
            systemPid)) == nullptr) {
            return false;
        }

        HANDLE hSystemToken;
        if (!OpenProcessToken(
            hSystemProcess,
            MAXIMUM_ALLOWED,
            &hSystemToken))
        {
            CloseHandle(hSystemProcess);
            return false;
        }

        HANDLE hDupToken;
        SECURITY_ATTRIBUTES tokenAttributes;
        tokenAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
        tokenAttributes.lpSecurityDescriptor = nullptr;
        tokenAttributes.bInheritHandle = FALSE;
        if (!DuplicateTokenEx(
            hSystemToken,
            MAXIMUM_ALLOWED,
            &tokenAttributes,
            SecurityImpersonation,
            TokenImpersonation,
            &hDupToken))
        {
            CloseHandle(hSystemToken);
            return false;
        }

        if (!ImpersonateLoggedOnUser(hDupToken))
        {
            CloseHandle(hDupToken);
            CloseHandle(hSystemToken);
            return false;
        }

        if (!SetThreadToken(0, hDupToken))
            return false;

        CloseHandle(hDupToken);
        CloseHandle(hSystemToken);

        return true;
    }

    DWORD StartTrusted() {
        auto sc_manager = OpenSCManagerA(
            nullptr,
            SERVICES_ACTIVE_DATABASE,
            GENERIC_EXECUTE
        );

        if (!sc_manager)
            return -1;

        auto service = OpenServiceA(
            sc_manager,
            "TrustedInstaller",
            GENERIC_READ | GENERIC_EXECUTE
        );

        if (!service)
        {
            CloseServiceHandle(sc_manager);
            return -1;
        }

        SERVICE_STATUS_PROCESS statusBuffer;
        DWORD bytesNeeded;
        while (QueryServiceStatusEx(
            service,
            SC_STATUS_PROCESS_INFO,
            reinterpret_cast<LPBYTE>(&statusBuffer),
            sizeof(SERVICE_STATUS_PROCESS),
            &bytesNeeded))
        {
            if (statusBuffer.dwCurrentState == SERVICE_STOPPED)
            {
                if (!StartServiceW(service, 0, nullptr))
                {
                    CloseServiceHandle(service);
                    CloseServiceHandle(sc_manager);
                    return -1;
                }
            }
            if (statusBuffer.dwCurrentState == SERVICE_START_PENDING ||
                statusBuffer.dwCurrentState == SERVICE_STOP_PENDING)
            {
                Sleep(statusBuffer.dwWaitHint);
                continue;
            }
            if (statusBuffer.dwCurrentState == SERVICE_RUNNING)
            {
                CloseServiceHandle(service);
                CloseServiceHandle(sc_manager);
                return statusBuffer.dwProcessId;
            }
        }
        CloseServiceHandle(service);
        CloseServiceHandle(sc_manager);

        return -1;
    }

    bool ProcessCreate(std::string commandLine) {
        auto pid = StartTrusted();

        EnablePrivilege(SE_DEBUG_NAME);
        EnablePrivilege(SE_IMPERSONATE_NAME);
        ImpersonateSystem();

        auto hTIProcess = OpenProcess(
            PROCESS_DUP_HANDLE | PROCESS_QUERY_INFORMATION,
            FALSE, pid
        );

        if (!hTIProcess)
            return false;

        HANDLE hTIToken;
        if (!OpenProcessToken(hTIProcess, MAXIMUM_ALLOWED, &hTIToken)) {
            CloseHandle(hTIProcess);
            return false;
        }

        HANDLE hDupToken;
        SECURITY_ATTRIBUTES tokenAttributes;
        tokenAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
        tokenAttributes.lpSecurityDescriptor = nullptr;
        tokenAttributes.bInheritHandle = FALSE;

        if (!DuplicateTokenEx(
            hTIToken,
            MAXIMUM_ALLOWED,
            &tokenAttributes,
            SecurityImpersonation,
            TokenImpersonation,
            &hDupToken
        )) {
            CloseHandle(hTIToken);
            return false;
        }

        STARTUPINFOW startupInfo;
        ZeroMemory(&startupInfo, sizeof(STARTUPINFOW));
        startupInfo.lpDesktop = (LPWSTR)L"Winsta0\\Default";
        PROCESS_INFORMATION processInfo;
        ZeroMemory(&processInfo, sizeof(PROCESS_INFORMATION));

        if (!CreateProcessWithTokenW(
            hDupToken,
            LOGON_WITH_PROFILE,
            nullptr,
            const_cast<LPWSTR>(Util::StringToWString(commandLine).c_str()),
            CREATE_UNICODE_ENVIRONMENT,
            nullptr,
            nullptr,
            &startupInfo,
            &processInfo
        ))
            return false;

        return true;
    }

    bool IsSystemGroup() {
        DWORD dw_size = 0;
        DWORD dw_result = 0;
        HANDLE token;
        PTOKEN_USER token_user;
        LPWSTR SID = NULL;

        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token))
            return false;

        if (!GetTokenInformation(token, TokenUser, NULL, dw_size, &dw_size)) {
            dw_result = GetLastError();
            if (dw_result != ERROR_INSUFFICIENT_BUFFER)
                return false;
        }

        token_user = (PTOKEN_USER)GlobalAlloc(GPTR, dw_size);

        if (!GetTokenInformation(token, TokenUser, token_user, dw_size, &dw_size))
            return false;

        if (!token_user)
            return false;

        if (!ConvertSidToStringSidW(token_user->User.Sid, &SID))
            return false;

        if (_wcsicmp(L"S-1-5-18", SID) == 0)
            return true;

        if (token_user)
            GlobalFree(token_user);

        return false;
    }

    bool HasAdmin() {
        BOOL ret = FALSE;
        HANDLE token = NULL;

        if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) {
            TOKEN_ELEVATION elevation;
            DWORD rlen = sizeof(TOKEN_ELEVATION);

            if (GetTokenInformation(token, TokenElevation, &elevation, sizeof(elevation), &rlen))
                ret = elevation.TokenIsElevated;
        }

        if (token)
            CloseHandle(token);

        return ret;
    }
}