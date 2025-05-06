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
#include "RulesStruct.h"
#include "Functions.h"
#include "IPFunctions.h"

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
        BOOL checkedServerVariable = FALSE;

        if (!myFunctions.GetIsEnabled(pHttpContext))
        {
#ifdef _DEBUG
            myFunctions.WriteFileLogMessage("Module disabled");
#endif
            return RQ_NOTIFICATION_CONTINUE;
        }

        // Perhaps you are using a module like Application Request Routing and need to base it off the value of HTTP_X_FORWARDED_FOR?
        if (myFunctions.CheckServerVariable(pHttpContext))
        {
            DWORD val;
            PCWSTR serverVariable;
            HRESULT hr = pHttpContext->GetServerVariable("HTTP_X_FORWARDED_FOR", &serverVariable, &val);
            if (SUCCEEDED(hr)) {
                _bstr_t b(serverVariable);
                PCSTR pcstrServerVariable = b;
#ifdef _DEBUG
                myFunctions.WriteFileLogMessage("Checking HTTP_X_FORWARDED_FOR variable instead");
                myFunctions.WriteFileLogMessage(pcstrServerVariable);
#endif
                INT family;
                hr = ipFunctions.GetIpVersion(pcstrServerVariable, &family);
                if (SUCCEEDED(hr)) {
                    hr = ipFunctions.StringToPSOCK(pcstrServerVariable, family, &pSockAddr);
                    if (SUCCEEDED(hr)) {
                        checkedServerVariable = TRUE;
                    }
                }
            }
        }
        // get exception rules from config, return only matching family
        const std::vector<ExceptionRules> rules = myFunctions.exceptionRules(pHttpContext, pSockAddr);
        // check exception rules
        BOOL allowed = FALSE;
        if (ipFunctions.isIpInExceptionRules(pSockAddr, rules, &allowed))
        {
            if (allowed)
            {
#ifdef _DEBUG
                myFunctions.WriteFileLogMessage("IP allowed by exception rule");
#endif
                if (checkedServerVariable) free(pSockAddr);
                return RQ_NOTIFICATION_CONTINUE;
            }
            else {
#ifdef _DEBUG
                myFunctions.WriteFileLogMessage("IP denied by exception rule");
#endif
                myFunctions.DenyAction(pHttpContext);
                if (checkedServerVariable) free(pSockAddr);
                return RQ_NOTIFICATION_FINISH_REQUEST;
            }
        }
#ifdef _DEBUG
        else {
            myFunctions.WriteFileLogMessage("No matching exception rules found for this address");
        }
#endif

        CHAR countryCode[3] = { '\0' }; // Buffer to store the country code (2 characters + null terminator)

        if (ipFunctions.IsLocalAddress(pSockAddr))
        {
#ifdef _DEBUG
            myFunctions.WriteFileLogMessage("address is local");
#endif
            strcpy_s(countryCode, "ZZ");
        }

        BOOL mode = myFunctions.GetAllowMode(pHttpContext);
        REQUEST_NOTIFICATION_STATUS reqStatus;

        // Get country code for this address, if it has not been set
        if (countryCode[0] == '\0')
        {
            CHAR* mmdbPath = myFunctions.GetMMDBPath(pHttpContext);
#ifdef _DEBUG
            myFunctions.WriteFileLogMessage(mmdbPath);
#endif
            HRESULT hr = myFunctions.GetCountryCode(pSockAddr, mmdbPath, countryCode);
            delete[] mmdbPath;
        }
        LPCWSTR wCountryCode = myFunctions.convertCharArrayToLPCWSTR(countryCode, 3);
        pHttpContext->SetServerVariable("GEOIP_COUNTRY", wCountryCode);
        delete[] wCountryCode;

        // check the retrieved country code
        if (myFunctions.CheckCountryCode(pHttpContext, countryCode, mode))
        {
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

        if(checkedServerVariable) free(pSockAddr);
        return reqStatus;
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