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
#define WIN32_LEAN_AND_MEAN
#include "pch.h"
#include <httpserv.h>
#include "Functions.h"
#include <Windows.h>

// IPv6 functions and date
#include <chrono>

#define ELEMENT L"system.webServer/CGeoIPModule"

HRESULT Functions::GetConfig(IHttpContext* pHttpContext, IAppHostElement** ppElement)
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

HRESULT GetStringPropertyValueFromElement(
    IAppHostElement* pElement,
    BSTR pszElementName,
    BSTR* pStringValue)
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

HRESULT
GetBooleanPropertyValueFromElement(
    IAppHostElement* pElement,
    BSTR pszElementName,
    BOOL* pBoolValue)
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
        return E_FAIL; // Value is not a string
    }

    // Finally, get the value:
    *pBoolValue = (vPropertyValue.boolVal == VARIANT_TRUE) ? TRUE : FALSE;

    VariantClear(&vPropertyValue);
    pProperty->Release();

    if (*pBoolValue == NULL)
    {
        return E_OUTOFMEMORY;
    }

    return hr;
}

BOOL Functions::IsCountryCodeListed(IHttpContext* pHttpContext, BSTR CountryCode)
{
    BOOL Allowed = FALSE;
    IAppHostElement* pModuleElement = NULL;

    HRESULT hr = GetConfig(pHttpContext, &pModuleElement);
    if (FAILED(hr))
    {
        return Allowed;
    }

    IAppHostElement* pCountryCodesElement = NULL;
    BSTR bstr = SysAllocString(L"CountryCodes");
    hr = pModuleElement->GetElementByName(bstr, &pCountryCodesElement);
    SysFreeString(bstr);
    pModuleElement->Release();
    if (FAILED(hr) || pCountryCodesElement == NULL)
    {
        return Allowed;
    }

    IAppHostElementCollection* pCollection = NULL;

    hr = pCountryCodesElement->get_Collection(&pCollection);
    pCountryCodesElement->Release();
    if (FAILED(hr) || pCollection == NULL)
    {
        return Allowed;
    }

    DWORD count = 0;
    hr = pCollection->get_Count(&count);
    if (FAILED(hr))
    {
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
            continue; // Move to the next element
        }

        BSTR bstrElementName = NULL;
        hr = pElement->get_Name(&bstrElementName);
        if (FAILED(hr) || bstrElementName == NULL)
        {
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
            if (SUCCEEDED(hr) && bstrCountryCode != NULL)
            {
                if (wcscmp(CountryCode, bstrCountryCode) == 0)
                {
#ifdef _DEBUG
                    WriteFileLogMessage("Found country code in config");
#endif
                    Allowed = TRUE;
                    SysFreeString(bstrCountryCode);
                    break;
                }
            }
            SysFreeString(bstrCountryCode);
        }

        SysFreeString(bstrElementName);
        pElement->Release();
    }

    pCollection->Release();

    return Allowed;
}


CHAR* Functions::GetMMDBPath(IHttpContext* pW3Context)
{
    BSTR bstr = NULL;
    IAppHostElement* pElement = NULL;
    HRESULT hr = GetConfig(pW3Context, &pElement);
    if (SUCCEEDED(hr))
    {
        BSTR path = SysAllocString(L"path");
        hr = GetStringPropertyValueFromElement(pElement, path, &bstr);
        pElement->Release();
        SysFreeString(path);
    }
    CHAR* path = BSTRToCharArray(bstr);
    SysFreeString(bstr);
    return path;
}

/// <summary>
/// true to allow only listed country codes
/// </summary>
/// <param name="pHttpContext"></param>
/// <returns></returns>
BOOL Functions::GetAllowMode(IHttpContext* pHttpContext)
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

BOOL Functions::GetIsEnabled(IHttpContext* pHttpContext)
{
    BOOL isEnabled = FALSE;
    IAppHostElement* pElement = NULL;
    HRESULT hr = GetConfig(pHttpContext, &pElement);
    if (SUCCEEDED(hr))
    {
        BSTR bstrEnabled = SysAllocString(L"enabled");
        hr = GetBooleanPropertyValueFromElement(pElement, bstrEnabled, &isEnabled);
        pElement->Release();
        SysFreeString(bstrEnabled);
    }
    return isEnabled;
}

/// <summary>
/// pick deny response
/// </summary>
/// <param name="pHttpContext"></param>
/// <returns></returns>
VOID Functions::DenyAction(IHttpContext* pHttpContext)
{
    // Default mode
    const wchar_t* mode = L"Forbidden";

    IAppHostElement* pElement = NULL;
    HRESULT hr = GetConfig(pHttpContext, &pElement); // Pass the desired section name
    if (SUCCEEDED(hr))
    {
        BSTR bstrAction = SysAllocString(L"action");
        BSTR modeBstr = NULL;

        // Retrieve the mode value from the configuration
        hr = GetStringPropertyValueFromElement(pElement, bstrAction, &modeBstr);
        pElement->Release(); // Release the retrieved element
        SysFreeString(bstrAction);

        if (SUCCEEDED(hr) && modeBstr != NULL)
        {
            mode = modeBstr;
            SysFreeString(modeBstr);
        }
    }

    IHttpResponse* pHttpResponse = pHttpContext->GetResponse();
    if (wcscmp(mode, L"Abort\0") == 0)
    {
        pHttpResponse->CloseConnection();
    }
    else if (wcscmp(mode, L"Not Found\0") == 0)
    {
        pHttpResponse->SetStatus(404, "Not Found");
    }
    else if (wcscmp(mode, L"Forbidden\0") == 0)
    {
        pHttpResponse->SetStatus(403, "Forbidden");
    }
    else if (wcscmp(mode, L"Unauthorized\0") == 0)
    {
        pHttpResponse->SetStatus(401, "Unauthorized");
    }
    else if (wcscmp(mode, L"Reset\0") == 0)
    {
        pHttpResponse->ResetConnection();
    }
    else if (wcscmp(mode, L"Teapot\0") == 0)
    {
        pHttpResponse->SetStatus(418, "I'm a teapot");
    }
    else
    {
        pHttpResponse->CloseConnection();
#ifdef _DEBUG
        WriteFileLogMessage("Action not recognised");
#endif
    }
}

CHAR* Functions::PSOCKADDRtoString(PSOCKADDR pSockAddr)
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

wchar_t* Functions::convertCharArrayToLPCWSTR(const char* charArray, int length)
{
    wchar_t* wString = new wchar_t[length];
    MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, length);
    return wString;
}

char* Functions::BSTRToCharArray(BSTR bstr)
{
    int length = WideCharToMultiByte(CP_UTF8, 0, bstr, -1, NULL, 0, NULL, NULL);
    char* charArray = new char[length];
    WideCharToMultiByte(CP_UTF8, 0, bstr, -1, charArray, length, NULL, NULL);
    return charArray;
}

char* Functions::FormatStringPSOCKADDR(const char* message, PSOCKADDR pSockAddr)
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
 * Logging functions
 *
 */

#ifdef _DEBUG
VOID Functions::WriteFileLogMessage(const char* szMsg)
{
    HANDLE hFile = CreateFileA("C:/folder/Module.log", GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    // date
    std::time_t currentTime = std::time(nullptr);
    char buffer[80];
    std::tm localTime;
    localtime_s(&localTime, &currentTime);
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &localTime);

    if (INVALID_HANDLE_VALUE != hFile)
    {
        DWORD dwWritten;
        SetFilePointer(hFile, 0, NULL, FILE_END);
        WriteFile(hFile, buffer, (DWORD)strlen(buffer), &dwWritten, NULL);
        WriteFile(hFile, " ", 1, &dwWritten, NULL);
        WriteFile(hFile, szMsg, (DWORD)strlen(szMsg), &dwWritten, NULL);
        WriteFile(hFile, "\r\n", 2, &dwWritten, NULL);
        CloseHandle(hFile);
    }
    return;
}
#endif
