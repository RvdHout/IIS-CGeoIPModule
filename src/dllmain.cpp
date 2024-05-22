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
#include <sal.h>
#include <httpserv.h>
#include <windows.h>
#include <winsock2.h>
#include <maxminddb.h>
#include "Functions.h"
#include "IPFunctions.h"

#pragma comment(lib, "WS2_32")
#pragma comment(lib, "maxminddb.lib")

IHttpServer* g_pHttpServer = NULL;
PVOID g_pModuleContext = NULL;

// Create the module class.
class CGeoIPModule : public CHttpModule
{
public:
    REQUEST_NOTIFICATION_STATUS
        OnBeginRequest(
            IN IHttpContext* pHttpContext,
            IN IHttpEventProvider* pProvider
        )
    {
        UNREFERENCED_PARAMETER(pProvider);
        IHttpRequest* pHttpRequest = pHttpContext->GetRequest();
        PSOCKADDR pSockAddr = pHttpRequest->GetRemoteAddress();
        Functions myFunctions;
        IPFunctions ipFunctions;

        if (!myFunctions.GetIsEnabled(pHttpContext)) {
#ifdef _DEBUG
            myFunctions.WriteFileLogMessage("Module disabled");
#endif
            return RQ_NOTIFICATION_CONTINUE;
        }

        CHAR countryCode[3] = { '\0' }; // Buffer to store the country code (2 characters + null terminator)

        if (ipFunctions.IsLocalAddress(pSockAddr)) {
#ifdef _DEBUG
            myFunctions.WriteFileLogMessage("address is local");
#endif
            strcpy_s(countryCode, "ZZ");
        }

        BOOL mode = myFunctions.GetAllowMode(pHttpContext);
        REQUEST_NOTIFICATION_STATUS reqStatus;

        // Get country code for this address, if it has not been set
        if (countryCode[0] == '\0') {
            CHAR* mmdbPath = myFunctions.GetMMDBPath(pHttpContext);
#ifdef _DEBUG
            myFunctions.WriteFileLogMessage(mmdbPath);
#endif
            HRESULT hr = GetCountryCode(pSockAddr, mmdbPath, countryCode);
            delete[] mmdbPath;
        }
        LPCWSTR wCountryCode = myFunctions.convertCharArrayToLPCWSTR(countryCode, 3);
        pHttpContext->SetServerVariable("GEOIP_COUNTRY", wCountryCode);
        delete[] wCountryCode;

        // check the retrieved country code
        if (CheckCountryCode(pHttpContext, countryCode, mode)) {
#ifdef _DEBUG
            myFunctions.WriteFileLogMessage("CountryCode allowed");
#endif
            reqStatus = RQ_NOTIFICATION_CONTINUE;
        }
        else {
#ifdef _DEBUG
            myFunctions.WriteFileLogMessage("CountryCode denied");
#endif
            myFunctions.DenyAction(pHttpContext);
            reqStatus = RQ_NOTIFICATION_FINISH_REQUEST;
        }
        
        return reqStatus;
    }

private:
    HRESULT
        GetCountryCode(
            IN PSOCKADDR IP,
            IN CHAR* MMDB_PATH,
            OUT CHAR* COUNTRYCODE
        )
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
    /// check if the countrycode associated with an ip is found in our allowed config
    /// </summary>
    /// <param name="COUNTRYCODE"></param>
    /// <returns>true if countrycode is one of the configured</returns>
    BOOL
        CheckCountryCode(
            IN IHttpContext* pHttpContext,
            IN CHAR* COUNTRYCODE,
            IN BOOL MODE
        )
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
            modeswitch = MODE ?  TRUE : FALSE;
        }

        SysFreeString(bstrCountryCode);

        return modeswitch;
    }
};

class MyHttpModuleFactory : public IHttpModuleFactory
{
public:
    HRESULT
        GetHttpModule(
            OUT CHttpModule** ppModule,
            IN IModuleAllocator* pAllocator
        )
    {
        UNREFERENCED_PARAMETER(pAllocator);
        CGeoIPModule* pModule = new CGeoIPModule;

        if (!pModule)
        {
            return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
        }
        else
        {
            *ppModule = pModule;
            pModule = NULL;
            return S_OK;
        }
    }

    void Terminate()
    {
        delete this;
    }
};

// Create the module's exported registration function.
HRESULT
__stdcall
RegisterModule(
    DWORD dwServerVersion,
    IHttpModuleRegistrationInfo* pModuleInfo,
    IHttpServer* pGlobalInfo
)
{
    UNREFERENCED_PARAMETER(dwServerVersion);

    g_pModuleContext = pModuleInfo->GetId();
    g_pHttpServer = pGlobalInfo;

    // Set the request notifications and exit.
    return pModuleInfo->SetRequestNotifications(
        // Specify the class factory.
        new MyHttpModuleFactory,
        // Specify the event notifications.
        RQ_BEGIN_REQUEST,
        // Specify the post-event notifications.
        0
    );
}