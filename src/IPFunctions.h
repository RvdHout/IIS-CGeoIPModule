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
#include <string>
#include <vector>
#include <cstdlib> // For atoi
#include <stdexcept> // For std::stoi
#include "RulesStruct.h"
#include <chrono>
#include <cstring>
#include <WS2tcpip.h>

class IPFunctions
{
public:
    BOOL IsLocalAddress(PSOCKADDR pSockAddr);

    BOOL isIpInExceptionRules(PSOCKADDR pSockAddr, const std::vector<ExceptionRules>& rules, BOOL* pAllowed);

    INT GetIpVersion(PCSTR ipAddress);

    PSOCKADDR StringToPSOCK(IN PCSTR string, IN INT family);

private:
    VOID GenerateIpv6Mask(int prefixLength, struct in6_addr* mask);

    BOOL IsIpv6InSubnet(struct in6_addr* addr, struct in6_addr* subnet, struct in6_addr* mask);

    BOOL IsIpv4InSubnet(DWORD ip, DWORD subnet, DWORD mask);
};