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
#include "IPFunctions.h"
#include <Windows.h>
#include <chrono>
#include <iphlpapi.h>
#include <cstring>

#pragma comment(lib, "Iphlpapi.lib")


BOOL IPFunctions::IsIpv4InSubnet(DWORD ip, DWORD subnet, DWORD mask)
{
    return (ip & mask) == (subnet & mask);
}

BOOL IPFunctions::IsIpv6InSubnet(struct in6_addr* addr, struct in6_addr* subnet, struct in6_addr* mask)
{
    for (int i = 0; i < 16; ++i) {
        if ((addr->s6_addr[i] & mask->s6_addr[i]) != subnet->s6_addr[i]) {
            return FALSE;
        }
    }
    return TRUE;
}

// Function to generate an IPv6 mask based on the prefix length
VOID IPFunctions::GenerateIpv6Mask(int prefixLength, struct in6_addr* mask)
{
    std::memset(mask, 0, sizeof(struct in6_addr)); // Initialize the mask to all zeros

    for (int i = 0; i < prefixLength / 8; ++i) {
        mask->s6_addr[i] = 0xFF;
    }
    if (prefixLength % 8 != 0) {
        mask->s6_addr[prefixLength / 8] = static_cast<uint8_t>(0xFF << (8 - (prefixLength % 8)));
    }
}

/// <summary>
/// Local addresses
/// 192.168.0.0/16
/// 10.0.0.0/8
/// 0.0.0.0/8
/// 172.16.0.0/12
/// 127.0.0.0/8
/// fe80::/10
/// ::1/128
/// </summary>
/// <param name="pSockAddr"></param>
/// <returns>true if local</returns>
BOOL IPFunctions::IsLocalAddress(PSOCKADDR pSockAddr) {
    if (pSockAddr->sa_family == AF_INET) {
        SOCKADDR_IN* sockaddr_in = (struct sockaddr_in*)pSockAddr;
        DWORD clientIp = sockaddr_in->sin_addr.S_un.S_addr;

        // List of IPv4 local subnets and masks
        struct {
            const char* subnet;
            const char* mask;
        } ipv4_local_subnets[] = {
            {"192.168.0.0", "255.255.0.0"},
            {"10.0.0.0", "255.0.0.0"},
            {"0.0.0.0", "255.0.0.0"},
            {"172.16.0.0", "255.240.0.0"},
            {"127.0.0.0", "255.0.0.0"}
        };

        // Check each local subnets
        for (const auto& net : ipv4_local_subnets) {
            struct in_addr ipv4_subnet = { 0 }, ipv4_mask = { 0 };
            inet_pton(AF_INET, net.subnet, &ipv4_subnet);
            inet_pton(AF_INET, net.mask, &ipv4_mask);

            DWORD subnet = ipv4_subnet.S_un.S_addr;
            DWORD mask = ipv4_mask.S_un.S_addr;

            if (IsIpv4InSubnet(clientIp, subnet, mask)) {
                return TRUE;
            }
        }
    }
    else if (pSockAddr->sa_family == AF_INET6) {
        SOCKADDR_IN6* sockaddr_in6 = (struct sockaddr_in6*)pSockAddr;
        struct in6_addr ipv6_subnet = {}, ipv6_mask = {};

        // Check for link-local address (fe80::/10)
        inet_pton(AF_INET6, "fe80::", &ipv6_subnet);
        GenerateIpv6Mask(10, &ipv6_mask);
        if (IsIpv6InSubnet(&sockaddr_in6->sin6_addr, &ipv6_subnet, &ipv6_mask)) {
            return TRUE;
        }

        ipv6_subnet = {}, ipv6_mask = {};
        // Check for link-local address (fc00::/7)
        inet_pton(AF_INET6, "fc00::", &ipv6_subnet);
        GenerateIpv6Mask(7, &ipv6_mask);
        if (IsIpv6InSubnet(&sockaddr_in6->sin6_addr, &ipv6_subnet, &ipv6_mask)) {
            return TRUE;
        }

        // Check for IPv6 localhost (::1)
        struct in6_addr ipv6_localhost = {};
        inet_pton(AF_INET6, "::1", &ipv6_localhost);
        if (memcmp(&sockaddr_in6->sin6_addr, &ipv6_localhost, sizeof(struct in6_addr)) == 0) {
            return TRUE;
        }
    }
    return FALSE;
}