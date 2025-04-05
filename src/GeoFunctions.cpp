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
#include "GeoFunctions.h"

/// <summary>
/// Gets the MMDB path from config, else nullptr
/// </summary>
/// <param name="pW3Context"></param>
/// <returns>Pointer to path</returns>
CHAR* GeoFunctions::GetMMDBPath(IN IHttpContext* pHttpContext, IN IAppHostElement* pModuleElement)
{
    Functions functions;
    BSTR bstrPath = SysAllocString(L"path");
    if (bstrPath == NULL)
    {
        return nullptr;
    }

    BSTR bstr = NULL;
    
    HRESULT hr = functions.GetStringPropertyValueFromElement(pModuleElement, bstrPath, &bstr);
    SysFreeString(bstrPath);
    if (FAILED(hr))
    {
#ifdef _DEBUG
        functions.WriteFileLogMessage("[Functions::GetMMDBPath]: GetStringPropertyValueFromElement failed");
        _com_error err(hr);
        LPCTSTR errMsg = err.ErrorMessage();
        functions.WriteFileLogMessage(CStringA(errMsg));
#endif
        return nullptr;
    }

    CHAR* path = functions.BSTRToCharArray(pHttpContext, bstr);
    SysFreeString(bstr);

    return path; // Caller must free this memory
}

static DWORD WINAPI WatchMMDBFile(LPVOID lpParam) {
    OutputDebugStringA("WatchMMDBFile started");

    CHAR filePath[MAX_PATH];
    strcpy_s(filePath, reinterpret_cast<const char*>(lpParam));

    CHAR dirPath[MAX_PATH];
    strcpy_s(dirPath, filePath);
    LPSTR lastSlash = strrchr(dirPath, '\\');
    if (lastSlash) *lastSlash = '\0';

    // watch the directory
    HANDLE hDir = CreateFileA(
        dirPath, FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL, OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS, NULL);

    if (hDir == INVALID_HANDLE_VALUE) {
        DWORD err = GetLastError();
        CHAR msg[256];
        sprintf_s(msg, sizeof(msg), "ERROR: CreateFileA failed! Code: %lu", err);
        OutputDebugStringA(msg);
        return 1;
    }

    char buffer[1024];
    DWORD bytesReturned;

    while (true) {
        memset(buffer, 0, sizeof(buffer));
        OutputDebugStringA("Waiting for directory change...");
        BOOL result = ReadDirectoryChangesW(
            hDir, buffer, sizeof(buffer), FALSE,
            FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE, &bytesReturned, NULL, NULL);

        if (!result) {
            DWORD err = GetLastError();
            CHAR msg[256];
            sprintf_s(msg, sizeof(msg), "ERROR: ReadDirectoryChangesW failed! Code: %lu", err);
            OutputDebugStringA(msg);
            continue;
        }

        OutputDebugStringA("File change detected!");

        FILE_NOTIFY_INFORMATION* pInfo = (FILE_NOTIFY_INFORMATION*)buffer;
        do {
            WCHAR wideFileName[MAX_PATH];
            wcsncpy_s(wideFileName, pInfo->FileName, pInfo->FileNameLength / sizeof(WCHAR));
            wideFileName[pInfo->FileNameLength / sizeof(WCHAR)] = L'\0';

            CHAR narrowFileName[MAX_PATH];
            WideCharToMultiByte(CP_ACP, 0, wideFileName, -1, narrowFileName, MAX_PATH, NULL, NULL);

            if (strstr(filePath, narrowFileName)) { // Check if it's the file we're watching
                OutputDebugStringA("Reloading mmdb");
                g_reloadNeeded = true;
            }
            else {
                OutputDebugStringA("File change detected, but not the target file.");
            }

            if (pInfo->NextEntryOffset == 0) break;
            pInfo = (FILE_NOTIFY_INFORMATION*)((LPBYTE)pInfo + pInfo->NextEntryOffset);
        } while (true);
    }

    CloseHandle(hDir);
    return 0;
}

HRESULT GeoFunctions::LoadMMDB(IN IHttpContext* pHttpContext, IN IAppHostElement* pModuleElement)
{
    Functions myFunctions;

    HANDLE hMutex = CreateMutex(NULL, FALSE, L"Global\\MMDB_Load_Mutex");
    if (hMutex == NULL)
    {
#ifdef _DEBUG
        myFunctions.WriteFileLogMessage("Failed to create mutex.");
#endif
        return E_HANDLE;
    }

    DWORD dwWaitResult = WaitForSingleObject(hMutex, INFINITE);
    if (dwWaitResult == WAIT_FAILED)
    {
#ifdef _DEBUG
        myFunctions.WriteFileLogMessage("Failed to acquire mutex.");
#endif
        CloseHandle(hMutex);
        return E_FAIL;
    }

    if (g_reloadNeeded) {
        MMDB_close(&g_mmdb);
        g_reloadNeeded = false;
    }
    CHAR* path = GetMMDBPath(pHttpContext, pModuleElement);
    PCWSTR appIdWString;
    HRESULT hr = myFunctions.GetSiteId(pHttpContext, &appIdWString);
    if (FAILED(hr))
    {
#ifdef _DEBUG
        myFunctions.WriteFileLogMessage("Failed to get temp path.");
#endif
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
        return E_FAIL;
    }

    CHAR tempPath[MAX_PATH];
    if (GetTempPathA(MAX_PATH, tempPath) == 0) {
#ifdef _DEBUG
        myFunctions.WriteFileLogMessage("Failed to get temp path.");
#endif
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
        return E_FAIL;
    }
    CHAR fullTempPath[MAX_PATH];
    LPCSTR tempFileName = "CGeoIPModule_mmdb_";

    _bstr_t bstr(appIdWString);
    LPCSTR appId = bstr;
    snprintf(fullTempPath, sizeof(fullTempPath), "%s%s%s", tempPath, tempFileName, appId);

    if (CopyFileA(path, fullTempPath, FALSE) == 0) {
#ifdef _DEBUG
        char message[256];
        sprintf_s(message, sizeof(message), "Failed to copy file to temp folder. %s", fullTempPath);
        myFunctions.WriteFileLogMessage(message);
#endif
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
        return E_FAIL;
    }

    int status = MMDB_open(fullTempPath, MMDB_MODE_MMAP, &g_mmdb);

    if (MMDB_SUCCESS != status) {
#ifdef _DEBUG
        CHAR message[256];
        sprintf_s(message, sizeof(message), "LoadMMDB() An error occured in MMDB_open %s", MMDB_strerror(status));
        myFunctions.WriteFileLogMessage(message);
#endif
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
        MMDB_close(&g_mmdb);
        return E_FAIL;
    }

    HANDLE hThread = CreateThread(NULL, 0, WatchMMDBFile, reinterpret_cast<LPVOID>(path), 0, NULL);
    if (hThread == NULL) {
#ifdef _DEBUG
        myFunctions.WriteFileLogMessage("CreateThread failed!");
#endif
        return E_FAIL;
    }
#ifdef _DEBUG
    else {
        myFunctions.WriteFileLogMessage("WatchMMDBFile thread started successfully.");
    }
#endif

    ReleaseMutex(hMutex);
    CloseHandle(hMutex);

    return S_OK;
}

HRESULT GeoFunctions::GetCountryCode(IN PSOCKADDR IP, OUT CHAR* COUNTRYCODE)
{
#ifdef _DEBUG
    Functions functions;
    LPSTR message = functions.FormatStringPSOCKADDR("Client address is", IP);
    functions.WriteFileLogMessage(message);
    delete[] message;
#endif
    MMDB_entry_data_s entry_data;
    int mmdb_error;
    // perform lookup
    MMDB_lookup_result_s result = MMDB_lookup_sockaddr(&g_mmdb, IP, &mmdb_error);
    // check it
    if (MMDB_SUCCESS != mmdb_error) {
#ifdef _DEBUG
        functions.WriteFileLogMessage(MMDB_strerror(mmdb_error));
#endif
        strcpy_s(COUNTRYCODE, 3, "--");
        return E_UNEXPECTED;
    }

    if (result.found_entry == false) {
#ifdef _DEBUG
        LPSTR message = functions.FormatStringPSOCKADDR("no entry in database for", IP);
        functions.WriteFileLogMessage(message);
        delete[] message;
#endif
        strcpy_s(COUNTRYCODE, 3, "--");
        return E_FAIL;
    }

    // get values
    int getValueResult = MMDB_get_value(&result.entry, &entry_data, "country", "iso_code", NULL);
    // sanity check
    if (entry_data.has_data == false || entry_data.type != MMDB_DATA_TYPE_UTF8_STRING) {
#ifdef _DEBUG
        LPSTR message = functions.FormatStringPSOCKADDR("No data for", IP);
        delete[] message;
#endif
        strcpy_s(COUNTRYCODE, 3, "--");
        return E_UNEXPECTED;
    }

    if (getValueResult != MMDB_SUCCESS) {
#ifdef _DEBUG
        LPSTR message = functions.FormatStringPSOCKADDR("MMDB_get_value failed", IP);
        functions.WriteFileLogMessage(message);
        delete[] message;
#endif
        strcpy_s(COUNTRYCODE, 3, "--");
        return E_UNEXPECTED;
    }

    // buffer for the country code
    CHAR cc[3];
    int sprintf_countrycode = 0;
    int sprintf_debugmessage = 0;
    sprintf_countrycode = sprintf_s(cc, sizeof(cc), "%.*s", entry_data.data_size, entry_data.utf8_string);
#ifdef _DEBUG
    CHAR string[16];
    sprintf_debugmessage = sprintf_s(string, sizeof(string), "country code %.*s", entry_data.data_size, entry_data.utf8_string);
#endif
    if (sprintf_debugmessage < 0 || sprintf_countrycode < 0) {
        strcpy_s(COUNTRYCODE, 3, "--");
        return E_UNEXPECTED;
    }
    else {
#ifdef _DEBUG
        functions.WriteFileLogMessage(string);
#endif
        strcpy_s(COUNTRYCODE, 3, cc);
        return S_OK;
    }

    return E_FAIL;
}
