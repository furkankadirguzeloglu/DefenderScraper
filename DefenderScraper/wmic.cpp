#include "wmic.hpp"

namespace wmic {
    helper::helper(std::string wnamespace, std::string wclass, std::string wmethod) {
        last_error = 0;
        hres = 0;
        loc_ptr = nullptr;
        service_ptr = nullptr;
        class_ptr = nullptr;
        param_def_ptr = nullptr;
        class_inst_ptr = nullptr;
        method_name = SysAllocString(Util::StringToWString(wmethod).c_str());
        class_name = SysAllocString(Util::StringToWString(wclass).c_str());
        class_name_s = wclass;
        hres = CoInitializeEx(0, COINIT_MULTITHREADED);

        if (FAILED(hres)) {
            last_error = 1;
            return;
        }

        hres = CoInitializeSecurity(
            NULL,
            -1,
            NULL,
            NULL,
            RPC_C_AUTHN_LEVEL_DEFAULT,
            RPC_C_IMP_LEVEL_IMPERSONATE,
            NULL,
            EOAC_NONE,
            NULL
        );

        if (FAILED(hres)) {
            last_error = 2;
            CoUninitialize();
            return;
        }

        hres = CoCreateInstance(CLSID_WbemLocator, 0,
            CLSCTX_INPROC_SERVER,
            IID_IWbemLocator, (LPVOID*)&loc_ptr);

        if (FAILED(hres)) {
            last_error = 3;
            CoUninitialize();
            return;
        }

        hres = loc_ptr->ConnectServer(
            _bstr_t(wnamespace.c_str()),
            0, 0, 0, 0, 0, 0, &service_ptr
        );

        if (FAILED(hres)) {
            last_error = 4;
            loc_ptr->Release();
            CoUninitialize();
            return;
        }

        hres = CoSetProxyBlanket(
            service_ptr,
            RPC_C_AUTHN_WINNT,
            RPC_C_AUTHZ_NONE,
            NULL,
            RPC_C_AUTHN_LEVEL_CALL,
            RPC_C_IMP_LEVEL_IMPERSONATE,
            NULL,
            EOAC_NONE
        );

        if (FAILED(hres)) {
            last_error = 5;
            service_ptr->Release();
            loc_ptr->Release();
            CoUninitialize();
            return;
        }

        hres = service_ptr->GetObjectA(class_name, 0, 0, &class_ptr, 0);
        hres = class_ptr->GetMethod(method_name, 0, &param_def_ptr, 0);
        hres = param_def_ptr->SpawnInstance(0, &class_inst_ptr);
    }

    helper::~helper() {
        SysFreeString(class_name);
        SysFreeString(method_name);

        if (class_ptr)
            class_ptr->Release();

        if (class_inst_ptr)
            class_inst_ptr->Release();

        if (param_def_ptr)
            param_def_ptr->Release();

        if (loc_ptr)
            loc_ptr->Release();

        if (service_ptr)
            service_ptr->Release();

        CoUninitialize();
    }

    int helper::get_last_error()
    {
        return last_error;
    }
}
