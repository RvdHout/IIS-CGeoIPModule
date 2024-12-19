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
#include <httpserv.h>
#include <vector>
#include <string>
#include <maxminddb.h>
#include <chrono>
#ifdef _DEBUG
#include <comdef.h> // for _com_error
#include <atlstr.h>
#endif
#include "RulesStruct.h"

#pragma comment(lib, "WS2_32")
#pragma comment(lib, "maxminddb.lib")

extern IHttpServer* g_pHttpServer;

class Functions
{
public:
    static HRESULT GetConfig(IN IHttpContext* pHttpContext, OUT IAppHostElement** ppElement);

    BOOL IsCountryCodeListed(IN IHttpContext* pHttpContext, IN BSTR CountryCode);

    BOOL GetIsEnabled(IN IHttpContext* pW3Context);

    CHAR* GetMMDBPath(IN IHttpContext* pW3Context);

    HRESULT GetCountryCode(IN PSOCKADDR IP, IN CHAR* MMDB_PATH, OUT CHAR* COUNTRYCODE);

    BOOL CheckCountryCode(IN IHttpContext* pHttpContext, IN CHAR* COUNTRYCODE, IN BOOL MODE);

    VOID DenyAction(IN IHttpContext* pHttpContext);

    BOOL GetAllowMode(IN IHttpContext* pW3Context);

    wchar_t* convertCharArrayToLPCWSTR(IN const char* charArray, IN int length);

    char* BSTRToCharArray(IN BSTR bstr);

    std::vector<ExceptionRules> exceptionRules(IN IHttpContext* pHttpContext, IN PSOCKADDR pAddress);

#ifdef _DEBUG
    CHAR* PSOCKADDRtoString(IN PSOCKADDR pSockAddr);

    char* FormatStringPSOCKADDR(IN const char* string, IN PSOCKADDR pSockAddr);

    static VOID WriteFileLogMessage(IN const char* szMsg);
#endif

};
