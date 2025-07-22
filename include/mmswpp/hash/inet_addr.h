
#ifndef MMSWPP_HASH_INET_ADDR_H_INCLUDED
#define MMSWPP_HASH_INET_ADDR_H_INCLUDED

#include <mmswpp/inet_addr.h>
#include <memepp/hash/std/hash.hpp>

#include <type_traits>

namespace std {

template <>
struct hash<mmswpp::inet_addr>
{
    size_t operator()(const mmswpp::inet_addr& addr) const
    {
        switch (addr.storage()) {
        case mmswpp::inet_addr::storage_type::inet4:
        {
            auto ptr = addr.get_in4_addr_ptr();
            if (ptr) {
                return hash<uint32_t>()(ptr->s_addr);
            }
        } break;
        case mmswpp::inet_addr::storage_type::inet6:
        {
            auto ptr = addr.get_in6_addr_ptr();
            if (ptr) {
                const uint8_t* addr_bytes = ptr->s6_addr;
                size_t hash_value = 14695981039346656037ull; // FNV-1a offset
                for (size_t idx = 0; idx < 16; ++idx) {
                    hash_value ^= addr_bytes[idx];
                    hash_value *= 1099511628211ull; // FNV-1a prime
                }
                return hash_value;
            }
        } break;
        case mmswpp::inet_addr::storage_type::string:
        {
            auto ptr = addr.get_str_ptr();
            if (ptr) {
                return hash<memepp::string>()(*ptr);
            }
        } break;
        default:
            return 0;
        }

        return 0; // Default case for unspecified or invalid addresses
    }
};

}

#endif // !MMSWPP_HASH_INET_ADDR_H_INCLUDED

