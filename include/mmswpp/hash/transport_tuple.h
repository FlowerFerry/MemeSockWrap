
#ifndef MMSWPP_HASH_TRANSPORT_TUPLE_H_INCLUDED
#define MMSWPP_HASH_TRANSPORT_TUPLE_H_INCLUDED

#include <mmswpp/transport_tuple.h>
#include <mmswpp/hash/inet_addr.h>

namespace std {

template <>
struct hash<mmswpp::transport_tuple>
{
    size_t operator()(const mmswpp::transport_tuple& _tuple) const noexcept
    {
        size_t addr1 = hash<mmswpp::inet_addr>()(_tuple.local_addr());
        size_t addr2 = hash<mmswpp::inet_addr>()(_tuple.remote_addr());
        size_t port1 = hash<uint16_t>()(_tuple.local_port());
        size_t port2 = hash<uint16_t>()(_tuple.remote_port());
        size_t ptrl  = hash<int>()(_tuple.protocol());
        return addr1 ^ (addr2 << 1) ^ (port1 << 2) ^ (port2 << 3) ^ (ptrl << 4);
    }
};

}

#endif // !MMSWPP_HASH_TRANSPORT_TUPLE_H_INCLUDED
