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
#ifndef EXCEPTION_RULES
#define EXCEPTION_RULES

#include <string>
#include <vector>

struct ExceptionRules {
    std::string family;
    std::string address;
    std::string mask;
    bool mode;

    ExceptionRules()
        : family(""), address(""), mask(""), mode(true) {}

    ExceptionRules(const std::string& fam, const std::string& addr, const std::string& msk, bool mod)
        : family(fam), address(addr), mask(msk), mode(mod) {}
};

// Declare rule as an external variable
extern ExceptionRules rule;

#endif // EXCEPTION_RULES