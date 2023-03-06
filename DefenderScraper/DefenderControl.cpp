#include "DefenderControl.hpp"

namespace DefenderControl {
    void ToggleTamper(bool enable) {
        HKEY hkey;
        if (Regedit::CreateRegeditistry("SOFTWARE\\Microsoft\\Windows Defender\\Features", hkey)) {
            if (enable) {
                Regedit::SetKeyval(hkey, "TamperProtection", 5);
            }
            else {
                Regedit::SetKeyval(hkey, "TamperProtection", 0);
            }
        }
    }

    void KillSmartScreen() {
        auto pid = Util::GetProcessID("smartscreen.exe");
        auto proc = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
        TerminateProcess(proc, 0);
        if (proc)
            CloseHandle(proc);
    }

    bool ManageSecurityService(bool enable, std::string service_name) {
        auto sc_manager = OpenSCManagerA(0, 0, SC_MANAGER_CONNECT);

        if (!sc_manager)
            return false;

        auto service = OpenServiceA(
            sc_manager,
            service_name.c_str(),
            enable ? SERVICE_ALL_ACCESS :
            (SERVICE_CHANGE_CONFIG | SERVICE_STOP | DELETE)
        );

        if (!service) {
            CloseServiceHandle(sc_manager);
            return false;
        }

        if (enable) {
            if (!ChangeServiceConfigA(
                service,
                SERVICE_NO_CHANGE,
                SERVICE_AUTO_START,
                SERVICE_NO_CHANGE,
                0, 0, 0, 0, 0, 0, 0
            )) {
                throw std::runtime_error("Failed to modify " + service_name + " " + std::to_string(GetLastError()));
                return false;
            }

            if (!StartServiceA(service, 0, NULL)) {
                throw std::runtime_error("Failed to start " + service_name);
                return false;
            }
        }
        else {
            SERVICE_STATUS scStatus;
            if (!ControlService(service, SERVICE_CONTROL_STOP, &scStatus)) {
                auto last_error = GetLastError();
                if (last_error == ERROR_SERVICE_NOT_ACTIVE)
                    return true;

                throw std::runtime_error(
                    "Failed to stop " + service_name + " " + std::to_string(last_error)
                );
                return false;
            }

            if (!ChangeServiceConfigA(
                service,
                SERVICE_NO_CHANGE,
                SERVICE_DEMAND_START,
                SERVICE_NO_CHANGE,
                0, 0, 0, 0, 0, 0, 0
            )) {
                throw std::runtime_error(
                    "Failed to modify " + service_name + " " + std::to_string(GetLastError())
                );

                return false;
            }
            Sleep(3000);
        }

        return true;
    }

    bool ManageSecurityCenter(bool enable) {
        HKEY hkey;
        if (Regedit::CreateRegeditistry("SYSTEM\\CurrentControlSet\\Services\\wscsvc", hkey)) {
            if (enable) {
                if (!Regedit::SetKeyval(hkey, "Start", 2)) {
                    return false;
                }
            }
            else {
                if (!Regedit::SetKeyval(hkey, "Start", 4)) {
                    return false;
                }
            }
        }

        return true;
    }

    bool ManageWinDefend(bool enable) {
        return ManageSecurityService(enable, "WinDefend");
    }

    bool DisableDefender() {
        HKEY hkey;
        if (Regedit::CreateRegeditistry("SOFTWARE\\Policies\\Microsoft\\Windows Defender", hkey)) {
            Regedit::SetKeyval(hkey, "DisableAntiSpyware", 1);
        }

        if (Regedit::CreateRegeditistry("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\Run", hkey)) {
            Regedit::SetKeyvalBinary(hkey, "SecurityHealth", 3);
        }

        if (Regedit::CreateRegeditistry("SOFTWARE\\Microsoft\\Windows Defender", hkey)) {
            Regedit::SetKeyval(hkey, "DisableAntiSpyware", 1);
        }

        if (Regedit::CreateRegeditistry("SYSTEM\\CurrentControlSet\\Services\\WinDefend", hkey)) {
            Regedit::SetKeyval(hkey, "Start", 3);
        }

        if (Regedit::CreateRegeditistry("SOFTWARE\\Microsoft\\Windows Defender\\Real-Time Protection", hkey)) {
            Regedit::SetKeyval(hkey, "DisableRealtimeMonitoring", 1);
        }

        auto helper = new wmic::helper(
            "Root\\Microsoft\\Windows\\Defender",
            "MSFT_MpPreference",
            "Set"
        );

        if (auto error = helper->get_last_error()) {
            return false;
        }

        helper->execute("EnableControlledFolderAccess", "Disabled");
        helper->execute("PUAProtection", "disable");
        helper->execute<BOOL>("DisableRealtimeMonitoring", wmic::variant_type::t_bool, TRUE);
        helper->execute<BOOL>("DisableBehaviorMonitoring", wmic::variant_type::t_bool, TRUE);
        helper->execute<BOOL>("DisableBlockAtFirstSeen", wmic::variant_type::t_bool, TRUE);
        helper->execute<BOOL>("DisableIOAVProtection", wmic::variant_type::t_bool, TRUE);
        helper->execute<BOOL>("DisablePrivacyMode", wmic::variant_type::t_bool, TRUE);
        helper->execute<BOOL>("SignatureDisableUpdateOnStartupWithoutEngine", wmic::variant_type::t_bool, TRUE);
        helper->execute<BOOL>("DisableArchiveScanning", wmic::variant_type::t_bool, TRUE);
        helper->execute<BOOL>("DisableIntrusionPreventionSystem", wmic::variant_type::t_bool, TRUE);
        helper->execute<BOOL>("DisableScriptScanning", wmic::variant_type::t_bool, TRUE);
        helper->execute<BOOL>("DisableAntiSpyware", wmic::variant_type::t_bool, TRUE);
        helper->execute<BOOL>("DisableAntiVirus", wmic::variant_type::t_bool, TRUE);
        helper->execute<uint8_t>("SubmitSamplesConsent", wmic::variant_type::t_uint8, 2);
        helper->execute<uint8_t>("MAPSReporting", wmic::variant_type::t_uint8, 0);
        helper->execute<uint8_t>("HighThreatDefaultAction", wmic::variant_type::t_uint8, 6);
        helper->execute<uint8_t>("ModerateThreatDefaultAction", wmic::variant_type::t_uint8, 6);
        helper->execute<uint8_t>("LowThreatDefaultAction", wmic::variant_type::t_uint8, 6);
        helper->execute<uint8_t>("SevereThreatDefaultAction", wmic::variant_type::t_uint8, 6);
        helper->execute<uint8_t>("ScanScheduleDay", wmic::variant_type::t_uint8, 8);
        delete helper;
        return true;
    }
}