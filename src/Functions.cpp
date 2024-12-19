/**
 *
 *   _____                _           _   _                    _    _           _               _ _          ___   ___
 *  / ____|              | |         | | | |             ____ | |  (_)         | |             | (_)        / _ \ / _ \
 * | |     _ __ ___  __ _| |_ ___  __| | | |__  _   _   / __ \| | ___ _ __ ___ | |__   ___  ___| |_  ___ __| (_) | (_) |
 * | |    | '__/ _ \/ _` | __/ _ \/ _` | | '_ \| | | | / / _` | |/ / | '_ ` _ \| '_ \ / _ \/ __| | |/ __/ _ \__, |\__, |
 * | |____| | |  __/ (_| | ||  __/ (_| | | |_) | |_| || | (_| |   <| | | | | | | |_) | (_) \__ \ | | (_|  __/ / /   / /
 *  \_____|_|  \___|\__,_|\__\___|\__,_| |_.__/ \__, | \ \__,_|_|\_\_|_| |_| |_|_.__/ \___/|___/_|_|\___\___|/_/   /_/
 *                                               __/ |  \____/
 *                                              |___/
 */
#include "pch.h"
#include "Functions.h"
#include <Windows.h>

#define ELEMENT L"system.webServer/CGeoIPModule"

HRESULT Functions::GetConfig(IN IHttpContext* pHttpContext, OUT IAppHostElement** ppElement)
{
    HRESULT hr = S_OK;
    IAppHostAdminManager* pAdminManager = NULL;
    IAppHostElement* pSessionTrackingElement = NULL;
    IAppHostPropertyException* pPropertyException = NULL;

    // Get the IAppHostAdminManager instance
    pAdminManager = g_pHttpServer->GetAdminManager();
    if (pAdminManager == NULL)
    {
        // Admin manager not available, cannot proceed
        return E_UNEXPECTED;
    }

    // Get the configuration path
    PCWSTR pszConfigPath = pHttpContext->GetMetadata()->GetMetaPath();
    BSTR bstrConfigPath = SysAllocString(pszConfigPath);
    BSTR bstrSectionName = SysAllocString(ELEMENT);

    hr = pAdminManager->GetAdminSection(bstrSectionName, bstrConfigPath, &pSessionTrackingElement);
    SysFreeString(bstrConfigPath);
    SysFreeString(bstrSectionName);
    if (FAILED(hr) || pSessionTrackingElement == NULL)
    {
        // Failed to retrieve the section, or the section is not found
        return hr;
    }

    // Return the retrieved element
    *ppElement = pSessionTrackingElement;

    return hr;
}

HRESULT GetStringPropertyValueFromElement(IAppHostElement* pElement, BSTR pszElementName, BSTR* pStringValue)
{
    HRESULT hr = S_OK;
    IAppHostProperty* pProperty = NULL;
    VARIANT vPropertyValue;

    if (
        (pElement == NULL) ||
        (pszElementName == NULL) ||
        (pStringValue == NULL)
        )
    {
        return E_INVALIDARG;
    }

    // Get the property object for the attribute within the specified element:
    hr = pElement->GetPropertyByName(
        pszElementName,
        &pProperty);

    if (FAILED(hr))
    {
        return hr;
    }

    if (pProperty == NULL)
    {
        return E_UNEXPECTED;
    }

    // Get the attribute value:
    VariantInit(&vPropertyValue);

    hr = pProperty->get_Value(&vPropertyValue);

    if (FAILED(hr))
    {
        pProperty->Release();
        return hr;
    }

    // Check if the value is a string
    if (vPropertyValue.vt != VT_BSTR)
    {
        VariantClear(&vPropertyValue);
        pProperty->Release();
        return E_FAIL; // Value is not a string
    }

    // Allocate memory for the string value and copy the value
    *pStringValue = SysAllocString(vPropertyValue.bstrVal);

    // Release resources
    VariantClear(&vPropertyValue);
    pProperty->Release();

    if (*pStringValue == NULL)
    {
        return E_OUTOFMEMORY; // Memory allocation failed
    }

    return hr;
}

HRESULT GetBooleanPropertyValueFromElement(IAppHostElement* pElement, BSTR pszElementName, BOOL* pBoolValue)
{
    HRESULT hr = S_OK;
    IAppHostProperty* pProperty = NULL;
    VARIANT vPropertyValue;

    if (
        (pElement == NULL) ||
        (pszElementName == NULL) ||
        (pBoolValue == NULL)
        )
    {
        return E_INVALIDARG;
    }

    hr = pElement->GetPropertyByName(
        pszElementName,
        &pProperty);

    if (FAILED(hr))
    {
        return hr;
    }

    if (pProperty == NULL)
    {
        return E_UNEXPECTED;
    }

    VariantInit(&vPropertyValue);

    hr = pProperty->get_Value(&vPropertyValue);

    if (FAILED(hr))
    {
        VariantClear(&vPropertyValue);
        pProperty->Release();
        return hr;
    }

    if (vPropertyValue.vt != VT_BOOL)
    {
        VariantClear(&vPropertyValue);
        pProperty->Release();
        return E_FAIL; // Value is not a bool
    }

    // Finally, get the value:
    *pBoolValue = (vPropertyValue.boolVal == VARIANT_TRUE) ? TRUE : FALSE;

    VariantClear(&vPropertyValue);
    pProperty->Release();

    return hr;
}

BOOL Functions::IsCountryCodeListed(IN IHttpContext* pHttpContext, IN BSTR CountryCode)
{
    BOOL Allowed = FALSE;
    IAppHostElement* pModuleElement = NULL;

    HRESULT hr = GetConfig(pHttpContext, &pModuleElement);
    if (FAILED(hr))
    {
#ifdef _DEBUG
        WriteFileLogMessage("[Functions::IsCountryCodeListed]: GetConfig failed");
        _com_error err(hr);
        LPCTSTR errMsg = err.ErrorMessage();
        WriteFileLogMessage(CStringA(errMsg));
#endif
        return Allowed;
    }

    IAppHostElement* pCountryCodesElement = NULL;
    BSTR bstr = SysAllocString(L"countryCodes");
    hr = pModuleElement->GetElementByName(bstr, &pCountryCodesElement);
    SysFreeString(bstr);
    pModuleElement->Release();
    if (FAILED(hr) || pCountryCodesElement == NULL)
    {
#ifdef _DEBUG
        WriteFileLogMessage("[Functions::IsCountryCodeListed]: GetElementByName failed");
        _com_error err(hr);
        LPCTSTR errMsg = err.ErrorMessage();
        WriteFileLogMessage(CStringA(errMsg));
#endif
        return Allowed;
    }

    IAppHostElementCollection* pCollection = NULL;

    hr = pCountryCodesElement->get_Collection(&pCollection);
    pCountryCodesElement->Release();
    if (FAILED(hr) || pCollection == NULL)
    {
#ifdef _DEBUG
        WriteFileLogMessage("[Functions::IsCountryCodeListed]: get_Collection failed");
        _com_error err(hr);
        LPCTSTR errMsg = err.ErrorMessage();
        WriteFileLogMessage(CStringA(errMsg));
#endif
        return Allowed;
    }

    DWORD count = 0;
    hr = pCollection->get_Count(&count);
    if (FAILED(hr))
    {
#ifdef _DEBUG
        WriteFileLogMessage("[Functions::IsCountryCodeListed]: get_Count failed");
        _com_error err(hr);
        LPCTSTR errMsg = err.ErrorMessage();
        WriteFileLogMessage(CStringA(errMsg));
#endif
        pCollection->Release();
        return Allowed;
    }

    // Iterate over each element in the collection
    for (DWORD i = 0; i < count; ++i)
    {
        VARIANT varIndex;
        VariantInit(&varIndex);
        varIndex.vt = VT_I4;
        varIndex.lVal = i;

        IAppHostElement* pElement = NULL;
        hr = pCollection->get_Item(varIndex, &pElement);
        if (FAILED(hr) || pElement == NULL)
        {
#ifdef _DEBUG
            WriteFileLogMessage("[Functions::IsCountryCodeListed]: get_Item failed");
            _com_error err(hr);
            LPCTSTR errMsg = err.ErrorMessage();
            WriteFileLogMessage(CStringA(errMsg));
#endif
            continue; // Move to the next element
        }

        BSTR bstrElementName = NULL;
        hr = pElement->get_Name(&bstrElementName);
        if (FAILED(hr) || bstrElementName == NULL)
        {
#ifdef _DEBUG
            WriteFileLogMessage("[Functions::IsCountryCodeListed]: get_Name failed");
            _com_error err(hr);
            LPCTSTR errMsg = err.ErrorMessage();
            WriteFileLogMessage(CStringA(errMsg));
#endif
            pElement->Release();
            continue; // Move to the next element
        }

        // Check if the element is an "add" element
        if (_wcsicmp(bstrElementName, L"add") == 0)
        {
            // Get the "code" attribute value
            BSTR bstrCountryCode = NULL;
            BSTR bstr = SysAllocString(L"code");
            hr = GetStringPropertyValueFromElement(pElement, bstr, &bstrCountryCode);
            SysFreeString(bstr);

            if (FAILED(hr) || bstrCountryCode == NULL)
            {
#ifdef _DEBUG
                WriteFileLogMessage("[Functions::IsCountryCodeListed]: GetStringPropertyValueFromElement failed");
                _com_error err(hr);
                LPCTSTR errMsg = err.ErrorMessage();
                WriteFileLogMessage(CStringA(errMsg));
#endif
                return Allowed;
            }

            if (wcscmp(CountryCode, bstrCountryCode) == 0)
            {
#ifdef _DEBUG
                WriteFileLogMessage("Found country code in config");
#endif
                Allowed = TRUE;
                SysFreeString(bstrCountryCode);
                break;
            }
            SysFreeString(bstrCountryCode);
        }

        SysFreeString(bstrElementName);
        pElement->Release();
    }

    pCollection->Release();

    return Allowed;
}

/// <summary>
/// Gets the MMDB path from config, else nullptr
/// </summary>
/// <param name="pW3Context"></param>
/// <returns>Pointer to path</returns>
CHAR* Functions::GetMMDBPath(IN IHttpContext* pW3Context)
{
    BSTR bstr = NULL;
    IAppHostElement* pElement = NULL;
    HRESULT hr = GetConfig(pW3Context, &pElement);
    if (FAILED(hr))
    {
#ifdef _DEBUG
        WriteFileLogMessage("[Functions::GetMMDBPath]: GetConfig failed");
        _com_error err(hr);
        LPCTSTR errMsg = err.ErrorMessage();
        WriteFileLogMessage(CStringA(errMsg));
#endif
        return nullptr;
    }

    BSTR bstrPath = SysAllocString(L"path");
    if (bstrPath == NULL)
    {
        pElement->Release();
        return nullptr;
    }

    hr = GetStringPropertyValueFromElement(pElement, bstrPath, &bstr);
    SysFreeString(bstrPath);
    if (FAILED(hr))
    {
#ifdef _DEBUG
        WriteFileLogMessage("[Functions::GetMMDBPath]: GetStringPropertyValueFromElement failed");
        _com_error err(hr);
        LPCTSTR errMsg = err.ErrorMessage();
        WriteFileLogMessage(CStringA(errMsg));
#endif
        pElement->Release();
        return nullptr;
    }

    CHAR* path = BSTRToCharArray(bstr);
    SysFreeString(bstr);
    pElement->Release();

    return path; // Caller must free this memory
}

/// <summary>
/// Gets the country code for an IP address
/// </summary>
/// <param name="IP">The clients PSOCKADDR</param>
/// <param name="MMDB_PATH">Path of mmdb file</param>
/// <param name="COUNTRYCODE">pointer with country code of ip</param>
/// <returns></returns>
HRESULT Functions::GetCountryCode(IN PSOCKADDR IP, IN CHAR* MMDB_PATH, OUT CHAR* COUNTRYCODE)
{
#ifdef _DEBUG
    Functions myFunctions;
    char* message = myFunctions.FormatStringPSOCKADDR("Client address is", IP);
    myFunctions.WriteFileLogMessage(message);
    delete[] message;
#endif
    MMDB_entry_data_s entry_data;
    MMDB_s mmdb;
    int mmdb_error;
    // open db
    int status = MMDB_open(MMDB_PATH, MMDB_MODE_MMAP, &mmdb);
    // check it
    if (MMDB_SUCCESS != status) {
#ifdef _DEBUG
        myFunctions.WriteFileLogMessage(MMDB_strerror(status));
#endif
        strcpy_s(COUNTRYCODE, 3, "--");
        MMDB_close(&mmdb);
        return E_UNEXPECTED;
    }
    // perform lookup
    MMDB_lookup_result_s result = MMDB_lookup_sockaddr(&mmdb, IP, &mmdb_error);
    // check it
    if (MMDB_SUCCESS != mmdb_error) {
#ifdef _DEBUG
        myFunctions.WriteFileLogMessage(MMDB_strerror(mmdb_error));
#endif
        strcpy_s(COUNTRYCODE, 3, "--");
        MMDB_close(&mmdb);
        return E_UNEXPECTED;
    }

    if (result.found_entry) {
        // get values
        int getValueResult = MMDB_get_value(&result.entry, &entry_data, "country", "iso_code", NULL);
        // sanity check
        if (!entry_data.has_data || entry_data.type != MMDB_DATA_TYPE_UTF8_STRING) {
#ifdef _DEBUG
            myFunctions.WriteFileLogMessage("No string data");
#endif
            strcpy_s(COUNTRYCODE, 3, "--");
            MMDB_close(&mmdb);
            return E_UNEXPECTED;
        }

        if (getValueResult == MMDB_SUCCESS) {
            // buffer for the country code
            char cc[3];
            int sprintf_countrycode = 0;
            int sprintf_debugmessage = 0;
            sprintf_countrycode = sprintf_s(cc, sizeof(cc), "%.*s", entry_data.data_size, entry_data.utf8_string);
#ifdef _DEBUG
            char string[16];
            sprintf_debugmessage = sprintf_s(string, sizeof(string), "country code %.*s", entry_data.data_size, entry_data.utf8_string);
#endif
            if (sprintf_debugmessage < 0 || sprintf_countrycode < 0) {
#ifdef _DEBUG
                char* message = myFunctions.FormatStringPSOCKADDR("Error formatting country code", IP);
                myFunctions.WriteFileLogMessage(message);
                delete[] message;
#endif
                strcpy_s(COUNTRYCODE, 3, "--");
                MMDB_close(&mmdb);
                return E_UNEXPECTED;
            }
            else {
#ifdef _DEBUG
                myFunctions.WriteFileLogMessage(string);
#endif
                strcpy_s(COUNTRYCODE, 3, cc);
                MMDB_close(&mmdb);
                return S_OK;
            }
        }
        else {
#ifdef _DEBUG
            char* message = myFunctions.FormatStringPSOCKADDR("MMDB_get_value failed", IP);
            myFunctions.WriteFileLogMessage(message);
            delete[] message;
#endif
            strcpy_s(COUNTRYCODE, 3, "--");
        }
    }
    else {
#ifdef _DEBUG
        char* message = myFunctions.FormatStringPSOCKADDR("no entry in database for", IP);
        myFunctions.WriteFileLogMessage(message);
        delete[] message;
#endif
        strcpy_s(COUNTRYCODE, 3, "--");
    }
    MMDB_close(&mmdb);
    return E_FAIL;
}


/// <summary>
/// Check country code returned by GetCountryCode
/// </summary>
/// <param name="pHttpContext">Context</param>
/// <param name="COUNTRYCODE">The country code of an IP</param>
/// <param name="MODE">mode switch</param>
/// <returns></returns>
BOOL Functions::CheckCountryCode(IN IHttpContext* pHttpContext, IN CHAR* COUNTRYCODE, IN BOOL MODE)
{
    Functions myFunctions;
#ifdef _DEBUG
    if (MODE == FALSE) {
        myFunctions.WriteFileLogMessage("mode=block listed");
    }
    else {
        myFunctions.WriteFileLogMessage("mode=allow listed");
    }
#endif
    BOOL modeswitch = MODE ? FALSE : TRUE;
#pragma warning( disable : 4267 )
    int wslen = MultiByteToWideChar(CP_ACP, 0, COUNTRYCODE, strlen(COUNTRYCODE), 0, 0);
    BSTR bstrCountryCode = SysAllocStringLen(0, wslen);
    MultiByteToWideChar(CP_ACP, 0, COUNTRYCODE, strlen(COUNTRYCODE), bstrCountryCode, wslen);
#pragma warning( default : 4267 )
    if (myFunctions.IsCountryCodeListed(pHttpContext, bstrCountryCode)) {
        modeswitch = MODE ? TRUE : FALSE;
    }

    SysFreeString(bstrCountryCode);

    return modeswitch;
}

/// <summary>
/// true to allow only listed country codes
/// </summary>
/// <param name="pHttpContext"></param>
/// <returns></returns>
BOOL Functions::GetAllowMode(IN IHttpContext* pHttpContext)
{
    BOOL mode = FALSE;
    IAppHostElement* pElement = NULL;
    HRESULT hr = GetConfig(pHttpContext, &pElement);
    if (SUCCEEDED(hr))
    {
        BSTR bstrEnabled = SysAllocString(L"allowListed");
        hr = GetBooleanPropertyValueFromElement(pElement, bstrEnabled, &mode);
        pElement->Release();
        SysFreeString(bstrEnabled);
    }
    return mode;
}

BOOL Functions::GetIsEnabled(IN IHttpContext* pHttpContext)
{
    BOOL isEnabled = FALSE;
    IAppHostElement* pElement = NULL;
    HRESULT hr = GetConfig(pHttpContext, &pElement);
    if (FAILED(hr))
    {
#ifdef _DEBUG
        WriteFileLogMessage("[Functions::GetIsEnabled]: GetConfig failed");
        _com_error err(hr);
        LPCTSTR errMsg = err.ErrorMessage();
        WriteFileLogMessage(CStringA(errMsg));
#endif
        return FALSE;
    }

    BSTR bstrEnabled = SysAllocString(L"enabled");
    if (bstrEnabled == NULL)
    {
#ifdef _DEBUG
        WriteFileLogMessage("[Functions::GetIsEnabled]: bstrEnabled alloc failed");
        _com_error err(hr);
        LPCTSTR errMsg = err.ErrorMessage();
        WriteFileLogMessage(CStringA(errMsg));
#endif
        pElement->Release();
        return FALSE;
    }

    hr = GetBooleanPropertyValueFromElement(pElement, bstrEnabled, &isEnabled);
    SysFreeString(bstrEnabled);
    if (FAILED(hr))
    {
#ifdef _DEBUG
        WriteFileLogMessage("[Functions::GetIsEnabled]: GetBooleanPropertyValueFromElement failed");
        _com_error err(hr);
        LPCTSTR errMsg = err.ErrorMessage();
        WriteFileLogMessage(CStringA(errMsg));
#endif
    }
    pElement->Release();
    return isEnabled;
}

/// <summary>
/// pick deny response, default to Close on failure
/// </summary>
/// <param name="pHttpContext"></param>
/// <returns></returns>
VOID Functions::DenyAction(IN IHttpContext* pHttpContext)
{
    const wchar_t* mode = L"Close"; // Default response on failure

    IAppHostElement* pElement = NULL;
    HRESULT hr = GetConfig(pHttpContext, &pElement);
    if (SUCCEEDED(hr))
    {
        BSTR bstrAction = SysAllocString(L"action");
        if (bstrAction != NULL)
        {
            BSTR modeBstr = NULL;
            hr = GetStringPropertyValueFromElement(pElement, bstrAction, &modeBstr);
            SysFreeString(bstrAction);
            pElement->Release();

            if (SUCCEEDED(hr) && modeBstr != NULL)
            {
                mode = modeBstr; // Use retrieved action
                SysFreeString(modeBstr);
            }
#ifdef _DEBUG
            else
            {
                WriteFileLogMessage("[Functions::DenyAction]: Failed to retrieve 'action' property");
                _com_error err(hr);
                LPCTSTR errMsg = err.ErrorMessage();
                WriteFileLogMessage(CStringA(errMsg));
            }
#endif
        }
        else
        {
#ifdef _DEBUG
            WriteFileLogMessage("[Functions::DenyAction]: SysAllocString for 'action' failed");
#endif
            pElement->Release();
        }
    }
#ifdef _DEBUG
    else
    {
        WriteFileLogMessage("[Functions::DenyAction]: GetConfig failed");
        _com_error err(hr);
        LPCTSTR errMsg = err.ErrorMessage();
        WriteFileLogMessage(CStringA(errMsg));
    }
#endif

    // Respond based on the mode
    IHttpResponse* pHttpResponse = pHttpContext->GetResponse();
    if (wcscmp(mode, L"Close") == 0)
    {
        pHttpResponse->CloseConnection();
    }
    else if (wcscmp(mode, L"Not Found") == 0)
    {
        pHttpResponse->SetStatus(404, "Not Found");
    }
    else if (wcscmp(mode, L"Forbidden") == 0)
    {
        pHttpResponse->SetStatus(403, "Forbidden");
    }
    else if (wcscmp(mode, L"Unauthorized") == 0)
    {
        pHttpResponse->SetStatus(401, "Unauthorized");
    }
    else if (wcscmp(mode, L"Reset") == 0)
    {
        pHttpResponse->ResetConnection();
    }
    else if (wcscmp(mode, L"Teapot") == 0)
    {
        pHttpResponse->SetStatus(418, "I'm a teapot");
    }
    else
    {
        pHttpResponse->CloseConnection();
#ifdef _DEBUG
        WriteFileLogMessage("[Functions::DenyAction]: Action not recognized, defaulting to 'Close'");
#endif
    }
}

std::vector<ExceptionRules> Functions::exceptionRules(IN IHttpContext* pHttpContext, IN PSOCKADDR pAddress) {
    std::vector<ExceptionRules> rules;
    IAppHostElement* pModuleElement = NULL;
    HRESULT hr = GetConfig(pHttpContext, &pModuleElement);
    if (FAILED(hr)) {
#ifdef _DEBUG
        WriteFileLogMessage("[Functions::exceptionRules]: GetConfig failed");
        _com_error err(hr);
        LPCTSTR errMsg = err.ErrorMessage();
        WriteFileLogMessage(CStringA(errMsg));
#endif
        return rules;
    }

    IAppHostElement* pAddressElement = NULL;
    BSTR bstr = SysAllocString(L"exceptionRules");
    hr = pModuleElement->GetElementByName(bstr, &pAddressElement);
    SysFreeString(bstr);
    pModuleElement->Release();

    if (FAILED(hr) || pAddressElement == NULL) {
#ifdef _DEBUG
        WriteFileLogMessage("[Functions::exceptionRules]: GetElementByName failed");
        _com_error err(hr);
        LPCTSTR errMsg = err.ErrorMessage();
        WriteFileLogMessage(CStringA(errMsg));
#endif
        return rules;
    }

    IAppHostElementCollection* pCollection = NULL;
    hr = pAddressElement->get_Collection(&pCollection);
    pAddressElement->Release();

    if (FAILED(hr) || pCollection == NULL) {
#ifdef _DEBUG
        WriteFileLogMessage("[Functions::exceptionRules]: get_Collection failed");
        _com_error err(hr);
        LPCTSTR errMsg = err.ErrorMessage();
        WriteFileLogMessage(CStringA(errMsg));
#endif
        return rules;
    }

    DWORD count = 0;
    hr = pCollection->get_Count(&count);
    if (FAILED(hr)) {
#ifdef _DEBUG
        WriteFileLogMessage("[Functions::exceptionRules]: get_Count failed");
        _com_error err(hr);
        LPCTSTR errMsg = err.ErrorMessage();
        WriteFileLogMessage(CStringA(errMsg));
#endif
        pCollection->Release();
        return rules;
    }

    for (DWORD i = 0; i < count; ++i) {
        VARIANT varIndex;
        VariantInit(&varIndex);
        varIndex.vt = VT_I4;
        varIndex.lVal = i;

        IAppHostElement* pElement = NULL;
        hr = pCollection->get_Item(varIndex, &pElement);
        if (FAILED(hr) || pElement == NULL) {
#ifdef _DEBUG
            WriteFileLogMessage("[Functions::exceptionRules]: get_Item failed");
            _com_error err(hr);
            LPCTSTR errMsg = err.ErrorMessage();
            WriteFileLogMessage(CStringA(errMsg));
#endif
            continue;
        }

        BSTR bstrElementName = NULL;
        hr = pElement->get_Name(&bstrElementName);
        if (FAILED(hr) || bstrElementName == NULL) {
#ifdef _DEBUG
            WriteFileLogMessage("[Functions::exceptionRules]: get_Name failed");
            _com_error err(hr);
            LPCTSTR errMsg = err.ErrorMessage();
            WriteFileLogMessage(CStringA(errMsg));
#endif
            pElement->Release();
            continue;
        }

        if (_wcsicmp(bstrElementName, L"add") == 0) {
            BSTR bstrFamily = NULL;
            BSTR family = SysAllocString(L"family");
            hr = GetStringPropertyValueFromElement(pElement, family, &bstrFamily);
            SysFreeString(family);

            if (FAILED(hr) || bstrFamily == NULL) {
#ifdef _DEBUG
                WriteFileLogMessage("[Functions::exceptionRules]: GetStringPropertyValueFromElement failed (family)");
                _com_error err(hr);
                LPCTSTR errMsg = err.ErrorMessage();
                WriteFileLogMessage(CStringA(errMsg));
#endif
                pElement->Release();
                continue;
            }

            // skip elements which are not for this address family, save some resources.
            if ((pAddress->sa_family == AF_INET && _wcsicmp(bstrFamily, L"ipv4") != 0) ||
                (pAddress->sa_family == AF_INET6 && _wcsicmp(bstrFamily, L"ipv6") != 0)) {
#ifdef _DEBUG
                WriteFileLogMessage("[Functions::exceptionRules]: Skipping entry family mismatch");
#endif
                SysFreeString(bstrFamily);
                pElement->Release();
                continue;
            }

            BOOL b = NULL;
            BSTR allow = SysAllocString(L"allow");
            hr = GetBooleanPropertyValueFromElement(pElement, allow, &b);
            SysFreeString(allow);
            bool boolMode = b;
            if (FAILED(hr)) {
#ifdef _DEBUG
                WriteFileLogMessage("[Functions::exceptionRules]: GetBooleanPropertyValueFromElement failed (allow)");
                _com_error err(hr);
                LPCTSTR errMsg = err.ErrorMessage();
                WriteFileLogMessage(CStringA(errMsg));
#endif
                SysFreeString(bstrFamily);
                pElement->Release();
                continue;
            }

            BSTR bstrMask = NULL;
            BSTR mask = SysAllocString(L"mask");
            hr = GetStringPropertyValueFromElement(pElement, mask, &bstrMask);
            SysFreeString(mask);

            if (FAILED(hr) || bstrMask == NULL) {
#ifdef _DEBUG
                WriteFileLogMessage("[Functions::exceptionRules]: GetStringPropertyValueFromElement failed (mask)");
#endif
                SysFreeString(bstrFamily);
                pElement->Release();
                continue;
            }

            BSTR bstrAddress = NULL;
            BSTR address = SysAllocString(L"address");
            hr = GetStringPropertyValueFromElement(pElement, address, &bstrAddress);
            SysFreeString(address);

            if (FAILED(hr) || bstrAddress == NULL) {
#ifdef _DEBUG
                WriteFileLogMessage("[Functions::exceptionRules]: GetStringPropertyValueFromElement failed (address)");
#endif
                SysFreeString(bstrFamily);
                SysFreeString(bstrMask);
                pElement->Release();
                continue;
            }

            // bstrAddress, bstrMask, bstrFamily, and mode contains config elements
            PCSTR pcstrAddress = BSTRToCharArray(bstrAddress);
            PCSTR pcstrMask = BSTRToCharArray(bstrMask);
            PCSTR pcstrFamily = BSTRToCharArray(bstrFamily);
            SysFreeString(bstrFamily);
            SysFreeString(bstrMask);
            SysFreeString(bstrAddress);

            rules.push_back({ std::string(pcstrFamily), std::string(pcstrAddress), std::string(pcstrMask), boolMode });

        }

        SysFreeString(bstrElementName);
        pElement->Release();
    }

    pCollection->Release();
    return rules;
}

wchar_t* Functions::convertCharArrayToLPCWSTR(IN const char* charArray, IN int length)
{
    wchar_t* wString = new wchar_t[length];
    MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, length);
    return wString;
}

char* Functions::BSTRToCharArray(IN BSTR bstr)
{
    int length = WideCharToMultiByte(CP_UTF8, 0, bstr, -1, NULL, 0, NULL, NULL);
    char* charArray = new char[length];
    WideCharToMultiByte(CP_UTF8, 0, bstr, -1, charArray, length, NULL, NULL);
    return charArray;
}

#ifdef _DEBUG

CHAR* Functions::PSOCKADDRtoString(IN PSOCKADDR pSockAddr)
{
    CHAR* string = nullptr;
    if (pSockAddr->sa_family == AF_INET) {
        char s[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(((struct sockaddr_in*)pSockAddr)->sin_addr), s, INET_ADDRSTRLEN);
        string = new CHAR[strlen(s) + 1];
        strcpy_s(string, strlen(s) + 1, s);
    }
    else if (pSockAddr->sa_family == AF_INET6) {
        char s[INET6_ADDRSTRLEN];
        inet_ntop(AF_INET6, &(((struct sockaddr_in6*)pSockAddr)->sin6_addr), s, INET6_ADDRSTRLEN);
        string = new CHAR[strlen(s) + 1];
        strcpy_s(string, strlen(s) + 1, s);
    }
    else {
        string = new CHAR[1];
        string[0] = '\0';
    }
    return string;
}

char* Functions::FormatStringPSOCKADDR(IN const char* message, IN PSOCKADDR pSockAddr)
{
    CHAR* ipstring = PSOCKADDRtoString(pSockAddr);
    size_t len = strlen(message) + strlen(ipstring) + 2; // 1 for space and 1 for null terminator
    char* result = new char[len];
    sprintf_s(result, len, "%s %s", message, ipstring);
    delete[] ipstring;
    return result;
}

/**
 *
 * Debug logging function
 *
 */

VOID Functions::WriteFileLogMessage(IN const char* szMsg)
{
    // Get system drive letter
    char* sysDrive = nullptr;
    size_t len = 0;
    if (_dupenv_s(&sysDrive, &len, "SystemDrive") != 0 || sysDrive == nullptr) {
        return;
    }

    // Construct path
    char path[MAX_PATH] = { 0 };
    snprintf(path, sizeof(path), "%s/inetpub/logs/CGeoIPDebugLog/Module.log", sysDrive);

    free(sysDrive);

    // Open file for writing
    HANDLE hFile = CreateFileA(path, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        return;
    }

    // date
    std::time_t currentTime = std::time(nullptr);
    char buffer[80];
    std::tm localTime;
    localtime_s(&localTime, &currentTime);
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &localTime);

    DWORD dwWritten;
    SetFilePointer(hFile, 0, NULL, FILE_END);
    WriteFile(hFile, buffer, (DWORD)strlen(buffer), &dwWritten, NULL);
    WriteFile(hFile, " ", 1, &dwWritten, NULL);
    WriteFile(hFile, szMsg, (DWORD)strlen(szMsg), &dwWritten, NULL);
    WriteFile(hFile, "\r\n", 2, &dwWritten, NULL);
    CloseHandle(hFile);
    return;
}
#endif
