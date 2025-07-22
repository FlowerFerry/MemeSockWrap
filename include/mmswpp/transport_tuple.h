
#ifndef MMSWPP_TRANSPORT_TUPLE_H_INCLUDED
#define MMSWPP_TRANSPORT_TUPLE_H_INCLUDED

#include "inet_addr.h"

namespace mmswpp {

struct transport_tuple
{

    transport_tuple()
        : local_addr_()
        , remote_addr_()
        , local_port_(0)
        , remote_port_(0)
        , protocol_(-1)
    {
    }

    transport_tuple(const inet_addr& _local, uint16_t _local_port, const inet_addr& _remote, uint16_t _remote_port, int _protocol)
        : local_addr_(_local)
        , remote_addr_(_remote)
        , local_port_(_local_port)
        , remote_port_(_remote_port)
        , protocol_(_protocol)
    {
    }

    transport_tuple(const transport_tuple&) = default;
    transport_tuple(transport_tuple&&) = default;
    transport_tuple& operator=(const transport_tuple&) = default;
    transport_tuple& operator=(transport_tuple&&) = default;

    bool operator==(const transport_tuple& _other) const noexcept
    {
        return local_addr_  == _other.local_addr_ &&
               remote_addr_ == _other.remote_addr_ &&
               local_port_  == _other.local_port_ &&
               remote_port_ == _other.remote_port_ &&
               protocol_    == _other.protocol_;
    }

    bool operator!=(const transport_tuple& _other) const noexcept
    {
        return !(*this == _other);
    }

    const inet_addr& local_addr() const noexcept
    {
        return local_addr_;
    }

    const inet_addr& remote_addr() const noexcept
    {
        return remote_addr_;
    }

    inet_addr& local_addr() noexcept
    {
        return local_addr_;
    }

    inet_addr& remote_addr() noexcept
    {
        return remote_addr_;
    }

    uint16_t local_port() const noexcept
    {
        return local_port_;
    }

    uint16_t remote_port() const noexcept
    {
        return remote_port_;
    }

    int protocol() const noexcept
    {
        return protocol_;
    }

    void set_local_addr(const inet_addr& _addr) noexcept
    {
        local_addr_ = _addr;
    }

    void set_remote_addr(const inet_addr& _addr) noexcept
    {
        remote_addr_ = _addr;
    }

    void set_local_port(uint16_t _port) noexcept
    {
        local_port_ = _port;
    }

    void set_remote_port(uint16_t _port) noexcept
    {
        remote_port_ = _port;
    }

    void set_protocol(int _protocol) noexcept
    {
        protocol_ = _protocol;
    }

    void set_local(const inet_addr& _addr, uint16_t _port) noexcept
    {
        local_addr_ = _addr;
        local_port_ = _port;
    }

    void set_remote(const inet_addr& _addr, uint16_t _port) noexcept
    {
        remote_addr_ = _addr;
        remote_port_ = _port;
    }

private:
    inet_addr local_addr_;
    inet_addr remote_addr_;
    uint16_t  local_port_;
    uint16_t  remote_port_;
    int protocol_;
};

}

#endif // !MMSWPP_TRANSPORT_TUPLE_H_INCLUDED
