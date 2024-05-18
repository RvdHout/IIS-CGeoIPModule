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
extern IHttpServer* g_pHttpServer;

class Functions
{
public:
    BOOL IsCountryCodeListed(IHttpContext* pHttpContext, BSTR CountryCode);

    BOOL GetIsEnabled(IHttpContext* pW3Context);

    CHAR* GetMMDBPath(IHttpContext* pW3Context);

    VOID DenyAction(IHttpContext* pHttpContext);

    BOOL IsLocalAddress(PSOCKADDR pSockAddr);

    CHAR* PSOCKADDRtoString(PSOCKADDR pSockAddr);

    BOOL GetAllowMode(IHttpContext* pW3Context);

    wchar_t* convertCharArrayToLPCWSTR(const char* charArray, int length);

    char* BSTRToCharArray(BSTR bstr);

    static HRESULT GetConfig(IHttpContext* pHttpContext, IAppHostElement** ppElement);

    char* FormatStringPSOCKADDR(const char* string, PSOCKADDR pSockAddr);

#ifdef _DEBUG
    static VOID WriteFileLogMessage(const char* szMsg);
#endif

private:
    BOOL IsIpv4InSubnet(DWORD ip, DWORD subnet, DWORD mask);

    VOID GenerateIpv6Mask(int prefixLength, struct in6_addr* mask);

    BOOL IsIpv6InSubnet(struct in6_addr* addr, struct in6_addr* subnet, struct in6_addr* mask);

};
