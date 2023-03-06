#pragma once
#include <iostream>
#include <Windows.h>
#include "Util.hpp"


#define _WIN32_DCOM
#include <comdef.h>
#include <Wbemidl.h>
#pragma comment(lib, "wbemuuid.lib")

namespace wmic {
  enum class variant_type : int {
    t_bool,
    t_bstr,
    t_uint8,
    t_uint32
  };

  class helper {
    int last_error;
    HRESULT hres;

    IWbemServices* service_ptr;
    IWbemLocator* loc_ptr;

    IWbemClassObject* class_ptr;
    IWbemClassObject* param_def_ptr;
    IWbemClassObject* class_inst_ptr;

    BSTR method_name;
    BSTR class_name;
    std::string class_name_s;

  public:

    helper(std::string wnamespace, std::string wclass, std::string wmethod);
    ~helper();
    int get_last_error();
    bool get(std::string variable, variant_type type, bstr_t& value) {
      IEnumWbemClassObject* pEnumerator = NULL;
      auto query = "SELECT * FROM " + class_name_s;
      auto hres = service_ptr->ExecQuery(
        bstr_t("WQL"),
        bstr_t(query.c_str()),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator
      );

      if (FAILED(hres))
        return false;

      IWbemClassObject* pclsObj = NULL;
      ULONG uReturn = 0;

      while (pEnumerator) {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1,
          &pclsObj, &uReturn);

        if (0 == uReturn)
          break;

        VARIANT vtProp = {};
        hr = pclsObj->Get(Util::StringToWString(variable).c_str(), 0, &vtProp, 0, 0);

        value = vtProp.bstrVal;

        VariantClear(&vtProp);
        pclsObj->Release();
      }

      return true;
    }

    template<typename T>
    bool get(std::string variable, variant_type type, T& value) {
      IEnumWbemClassObject* pEnumerator = NULL;
      auto query = "SELECT * FROM " + class_name_s;
      auto hres = service_ptr->ExecQuery(
        bstr_t("WQL"),
        bstr_t(query.c_str()),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator
      );

      if (FAILED(hres))
        return false;

      IWbemClassObject* pclsObj = NULL;
      ULONG uReturn = 0;

      while (pEnumerator) {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1,
          &pclsObj, &uReturn);

        if (0 == uReturn)
          break;

        VARIANT vtProp = {};
        hr = pclsObj->Get(Util::StringToWString(variable).c_str(), 0, &vtProp, 0, 0);

        switch (type) {
        case variant_type::t_bool:
          value = vtProp.boolVal;
          break;

        case variant_type::t_uint8:
          value = vtProp.uintVal;
          break;

        case variant_type::t_uint32:
          value = vtProp.uintVal;
          break;

        default:
          last_error = 6;
          return false;
        }

        VariantClear(&vtProp);
        pclsObj->Release();
      }

      return true;
    }

    void execute(std::string variable, std::string value) {
      VARIANT var_cmd;
      var_cmd.vt = VT_BSTR;
      var_cmd.bstrVal = _bstr_t(Util::StringToWString(value).c_str());
      hres = class_inst_ptr->Put(Util::StringToWString(variable).c_str(), 0, &var_cmd, 0);
      IWbemClassObject* pOutParams = nullptr;
      hres = service_ptr->ExecMethod(class_name, method_name, 0,
        0, class_inst_ptr, &pOutParams, 0);

      if (FAILED(hres))
        last_error = 7;

      VariantClear(&var_cmd);

      if (pOutParams)
        pOutParams->Release();
    }

    template<typename T>
    void execute(std::string variable, variant_type type, T value) {
      VARIANT var_cmd;

      switch (type) {
      case variant_type::t_bool:
        var_cmd.vt = VT_BOOL;
        var_cmd.boolVal = value;
        break;

      case variant_type::t_uint8:
        var_cmd.vt = VT_UI1;
        var_cmd.uintVal = value;
        break;

      case variant_type::t_uint32:
        var_cmd.vt = VT_UI4;
        var_cmd.uintVal = value;
        break;

      default:
        last_error = 6;
        return;
      }

      hres = class_inst_ptr->Put(Util::StringToWString(variable).c_str(), 0, &var_cmd, 0);
      IWbemClassObject* pOutParams = nullptr;
      hres = service_ptr->ExecMethod(class_name, method_name, 0,
        0, class_inst_ptr, &pOutParams, 0);

      if (FAILED(hres))
        last_error = 7;

      VariantClear(&var_cmd);

      if (pOutParams)
        pOutParams->Release();
    }
  };
}

