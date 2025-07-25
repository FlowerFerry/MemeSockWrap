
#ifndef MMSWPP_INET_ADDR_H_INCLUDED
#define MMSWPP_INET_ADDR_H_INCLUDED

#include <mego/predef/os/windows.h>
#include <mego/predef/os/linux.h>
#include <mego/predef/has_include.h>

#include <memepp/string.hpp>
#include <memepp/string_view.hpp>
#include <memepp/convert/self.hpp>
#include <megopp/util/scope_cleanup.h>

#if MG_OS__WIN_AVAIL
#include <winsock2.h>
#include <ws2tcpip.h>
#endif 

#if MG_OS__LINUX_AVAIL
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#if MG_HAS_INCLUDE(<mpark/variant.hpp>)
#include <mpark/variant.hpp>
#else
#include <variant>
#endif

#include <tuple>
#include <variant>

namespace mmswpp {
#if MG_HAS_INCLUDE(<mpark/variant.hpp>)
    namespace var_ns = mpark;
#else
    namespace var_ns = std;
#endif

struct inet_addr
{
    enum class family_type
    {
        unspecified = AF_UNSPEC,
        ipv4        = AF_INET,
        ipv6        = AF_INET6,
        custom      = 100000,
        ipv4_mapped = custom + AF_INET6, // Custom value for mapped IPv4 addresses
        domain      = custom + 100001,
    };

    enum class storage_type : uint8_t
    {
        invalid,
        string,
        inet4,
        inet6,
    };

    using in4_addr = ::in_addr;
    using in6_addr = ::in6_addr;

    using address_storage = var_ns::variant<
        var_ns::monostate, // Invalid address
        memepp::string, // String representation
        in4_addr,       // IPv4 address
        in6_addr        // IPv6 address
    >;

    inet_addr()
        : addr_(var_ns::monostate{})
        , family_(family_type::unspecified)
    {
    }

    inet_addr(const in4_addr& _addr)
        : addr_(_addr)
        , family_(family_type::ipv4)
    {
    }

    inet_addr(const in6_addr& _addr)
        : addr_(_addr)
        , family_(family_type::ipv6)
    {
        if (is_ipv4_mapped(_addr))
            family_ = family_type::ipv4_mapped;
    }

    explicit inet_addr(const char* _str)
    {
        assign(_str);
    }

    inet_addr(const memepp::string& _str)
        : addr_(_str)
        , family_(identify_family(_str))
    {
    }

    inet_addr(const inet_addr&) = default;
    inet_addr(inet_addr&&) = default;
    inet_addr& operator=(const inet_addr&) = default;
    inet_addr& operator=(inet_addr&&) = default;

    inet_addr& operator=(const in4_addr& _addr)
    {
        assign(_addr);
        return *this;
    }

    inet_addr& operator=(const in6_addr& _addr)
    {
        assign(_addr);
        return *this;
    }

    inet_addr& operator=(const char* _str)
    {
        assign(_str);
        return *this;
    }

    inet_addr& operator=(const memepp::string& _str)
    {
        assign(_str);
        return *this;
    }

    bool operator==(const inet_addr& _other) const
    {
        if (storage() == _other.storage())
        {
            switch (storage()) {
            case storage_type::inet4:
                return var_ns::get<in4_addr>(addr_).s_addr == var_ns::get<in4_addr>(_other.addr_).s_addr;
            case storage_type::inet6:
                return memcmp(var_ns::get<in6_addr>(addr_).s6_addr, var_ns::get<in6_addr>(_other.addr_).s6_addr, 16) == 0;
            case storage_type::string:
                return string_equal(
                    var_ns::get<memepp::string>(addr_), family_, var_ns::get<memepp::string>(_other.addr_), _other.family_);
            default:
                return true; // Both are invalid or unspecified
            }
        }

        switch (storage()) {
        case storage_type::inet4:
        {
            auto [addr4, valid] = _other.to_in4_addr();
            if (!valid) {
                return false;
            }
            return var_ns::get<in4_addr>(addr_).s_addr == addr4.s_addr;
        } break;
        case storage_type::inet6:
        {
            auto [addr6, valid] = _other.to_in6_addr();
            if (!valid) {
                return false;
            }
            return memcmp(var_ns::get<in6_addr>(addr_).s6_addr, addr6.s6_addr, 16) == 0;
        } break;
        case storage_type::string:
        {
            auto str = _other.to_string();
            return string_equal(
                var_ns::get<memepp::string>(addr_), family_, str, _other.family_);
        } break;
        default:
            return false; // Different storage types
        }
    }

    bool operator!=(const inet_addr& _other) const noexcept
    {
        return !(*this == _other);
    }

    family_type family() const noexcept
    {
        return family_;
    }

    storage_type storage() const noexcept
    {
        return static_cast<storage_type>(addr_.index());
    }

    bool is_ipv4() const noexcept
    {
        return family_ == family_type::ipv4;
    }

    bool is_ipv6() const noexcept
    {
        return family_ == family_type::ipv6 || family_ == family_type::ipv4_mapped;
    }

    bool is_domain() const noexcept
    {
        return family_ == family_type::domain;
    }

    bool is_ipv4_mapped() const noexcept
    {
        return family_ == family_type::ipv4_mapped;
    }

    bool is_unspecified() const noexcept
    {
        return family_ == family_type::unspecified;
    }

    bool is_ip() const noexcept
    {
        return is_ipv4() || is_ipv6();
    }

    std::tuple<in4_addr, bool> to_in4_addr() const noexcept
    {
        if (is_ipv4()) {
            if (storage() == storage_type::inet4) {
                return { var_ns::get<in4_addr>(addr_), true };
            }
            if (storage() == storage_type::string) {
                in4_addr addr;
                if (inet_pton(AF_INET, var_ns::get<memepp::string>(addr_).c_str(), &addr) == 1) 
                    return { addr, true };
            }
        }
        
        if (is_ipv4_mapped()) {
            if (storage() == storage_type::inet6) {
                in4_addr addr;
                std::memcpy(&addr, &(var_ns::get<in6_addr>(addr_).s6_addr[12]), sizeof(in4_addr));
                return { addr, true };
            }

            if (storage() == storage_type::string) {
                in6_addr addr6;
                if (inet_pton(AF_INET6, var_ns::get<memepp::string>(addr_).c_str(), &addr6) == 1) 
                {
                    in4_addr addr4;
                    std::memcpy(&addr4, &(addr6.s6_addr[12]), sizeof(in4_addr));
                    return { addr4, true };
                }
            }
        }

        in4_addr addr;
        memset(&addr, 0, sizeof(in4_addr));
        return { addr, false };
    }

    std::tuple<in6_addr, bool> to_in6_addr() const noexcept
    {
        if (is_ipv6()) {
            if (storage() == storage_type::inet6) {
                return { var_ns::get<in6_addr>(addr_), true };
            }
            if (storage() == storage_type::string) {
                in6_addr addr;
                if (inet_pton(AF_INET6, var_ns::get<memepp::string>(addr_).c_str(), &addr) == 1) 
                    return { addr, true };
            }
        }

        in6_addr addr;
        memset(&addr, 0, sizeof(in6_addr));
        return { addr, false };
    }

    bool fill_sockaddr_storage(uint16_t _port, sockaddr_storage& _out) const noexcept
    {
        if (!is_ip())
            return false;

        if (is_ipv4() || is_ipv4_mapped()) {
            sockaddr_in* addr = reinterpret_cast<sockaddr_in*>(&_out);
            addr->sin_family = AF_INET;
            addr->sin_port  = htons(_port);
            auto [addr4, valid] = to_in4_addr();
            if (!valid)
                return false;
            addr->sin_addr = addr4;
        } else if (is_ipv6()) {
            sockaddr_in6* addr = reinterpret_cast<sockaddr_in6*>(&_out);
            addr->sin6_family = AF_INET6;
            addr->sin6_port  = htons(_port);
            auto [addr6, valid] = to_in6_addr();
            if (!valid)
                return false;
            addr->sin6_addr = addr6;
        } else {
            return false; // Unsupported address family
        }
        return true;
    }

    memepp::string to_string() const
    {
        switch (static_cast<storage_type>(addr_.index())) 
        {
        case storage_type::inet4:
        {
            char buffer[INET_ADDRSTRLEN + 1];
            if (inet_ntop(AF_INET, &var_ns::get<in4_addr>(addr_), buffer, INET_ADDRSTRLEN) == nullptr)
            {
                return memepp::string{};
            }
            return memepp::string{ buffer };
        }
        case storage_type::inet6:
        {
            char buffer[INET6_ADDRSTRLEN + 1];
            if (inet_ntop(AF_INET6, &var_ns::get<in6_addr>(addr_), buffer, INET6_ADDRSTRLEN) == nullptr)
            {
                return memepp::string{};
            }
            return memepp::string{ buffer };
        }
        case storage_type::string:
        {
            return var_ns::get<memepp::string>(addr_);
        }
        default: {
            return memepp::string{};
        }
        }
    }

    const in4_addr* get_in4_addr_ptr() const noexcept
    {
        return var_ns::get_if<in4_addr>(&addr_);
    }

    const in6_addr* get_in6_addr_ptr() const noexcept
    {
        return var_ns::get_if<in6_addr>(&addr_);
    }

    const memepp::string* get_str_ptr() const noexcept
    {
        return var_ns::get_if<memepp::string>(&addr_);
    }

    void assign(const in4_addr& _addr)
    {
        addr_   = _addr;
        family_ = family_type::ipv4;
    }

    void assign(const in6_addr& _addr)
    {
        addr_   = _addr;
        family_ = identify_family(_addr);
    }

    void assign(const char* _str)
    {
        addr_   = memepp::string{ _str };
        family_ = identify_family(_str);
    }

    void assign(const memepp::string& _str)
    {
        addr_   = _str.trim_space();
        family_ = identify_family(var_ns::get<memepp::string>(addr_));
    }

    bool convert(storage_type _to_storage)
    {
        if (_to_storage == storage_type::invalid)
            return false;
        if (_to_storage == storage())
            return true;

        switch (_to_storage) {
        case storage_type::inet4:
        {
            if (storage() == storage_type::inet6 &&
                family() == family_type::ipv4_mapped)
            {
                in4_addr addr;
                std::memcpy(&addr, &(var_ns::get<in6_addr>(addr_).s6_addr[12]), sizeof(in4_addr));
                addr_ = addr;
                family_ = family_type::ipv4;
                return true;
            }

            if (storage() == storage_type::string)
            {
                if (family() == family_type::ipv4)
                {
                    in4_addr addr;
                    if (inet_pton(AF_INET, var_ns::get<memepp::string>(addr_).c_str(), &addr) != 1)
                    {
                        return false;
                    }
                    addr_   = addr;
                    family_ = family_type::ipv4;
                    return true;
                }

                if (family() == family_type::ipv4_mapped)
                {
                    in6_addr addr6;
                    if (inet_pton(AF_INET6, var_ns::get<memepp::string>(addr_).c_str(), &addr6) != 1)
                    {
                        return false;
                    }
                    in4_addr addr4;
                    std::memcpy(&addr4, &(addr6.s6_addr[12]), sizeof(in4_addr));
                    addr_   = addr4;
                    family_ = family_type::ipv4_mapped;
                    return true;
                }
            }

            break;
        }
        case storage_type::inet6:
        {
            if (storage() == storage_type::inet4)
            {
                addr_   = make_ipv4_mapped(var_ns::get<in4_addr>(addr_));
                family_ = family_type::ipv4_mapped;
                return true;
            }

            if (storage() == storage_type::string)
            {
                if (family() == family_type::ipv6)
                {
                    in6_addr addr;
                    if (inet_pton(AF_INET6, var_ns::get<memepp::string>(addr_).c_str(), &addr) != 1)
                    {
                        return false;
                    }
                    addr_   = addr;
                    family_ = family_type::ipv6;
                    return true;
                }

                if (family() == family_type::ipv4_mapped)
                {
                    in4_addr addr4;
                    if (inet_pton(AF_INET, var_ns::get<memepp::string>(addr_).c_str(), &addr4) != 1)
                    {
                        return false;
                    }
                    addr_   = make_ipv4_mapped(addr4);
                    family_ = family_type::ipv4_mapped;
                    return true;
                }
            }

            break;
        }
        case storage_type::string:
        {
            addr_ = to_string();
            return true;
        }
        default:
            break;
        }

        return false;
    }

    template<typename _Container>
    inline void split_domain(std::back_insert_iterator<_Container> _out) const
    {
        if (!is_domain())
            return;

        auto& str = var_ns::get<memepp::string>(addr_);
        str.split(".", _out);
    }

    static bool is_ipv4_mapped(const in6_addr& _addr)
    {
        for (int idx = 0; idx < 10; ++idx)
        {
            if (_addr.s6_addr[idx] != 0)
                return false;
        }
        return _addr.s6_addr[10] == 0xFF && _addr.s6_addr[11] == 0xFF;
    }

    static family_type identify_family(const in6_addr& _addr)
    {
        if (is_ipv4_mapped(_addr))
            return family_type::ipv4_mapped;
        return family_type::ipv6;
    }

    static family_type identify_family(const in4_addr& _addr)
    {
        return family_type::ipv4;
    }

    static family_type identify_family(const char* _str)
    {
        if (_str[0] == '\0')
            return family_type::unspecified;

        char buffer[sizeof(in6_addr)];
        if (inet_pton(AF_INET, _str, buffer) == 1)
            return family_type::ipv4;

        if (inet_pton(AF_INET6, _str, buffer) == 1)
        {
            return strncmp(buffer, "::ffff:", 7) == 0 || strncmp(buffer, "::FFFF:", 7) == 0 ?
                family_type::ipv4_mapped : family_type::ipv6;
        }

        return family_type::domain;
    }

    static family_type identify_family(const memepp::string& _str)
    {
        return identify_family(_str.c_str());
    }

    static in6_addr make_ipv4_mapped(const in4_addr& _addr)
    {
        in6_addr addr;
        memset(&addr, 0, sizeof(addr));
        addr.s6_addr[10] = 0xFF;
        addr.s6_addr[11] = 0xFF;
        std::memcpy(&addr.s6_addr[12], &_addr, sizeof(in4_addr));
        return addr;
    }

    static bool string_equal(
        const memepp::string_view& _lhs, family_type _lhs_family, 
        const memepp::string_view& _rhs, family_type _rhs_family) noexcept
    {
        if (_lhs.empty() && _rhs.empty())
            return true;
        if (_lhs.empty() || _rhs.empty())
            return false;

        if (_lhs_family == family_type::ipv4 && _rhs_family == family_type::ipv4_mapped)
        {
            in4_addr lhs_addr4;
            if (inet_pton(AF_INET, _lhs.data(), &lhs_addr4) != 1)
                return false;
            in6_addr rhs_addr6;
            if (inet_pton(AF_INET6, _rhs.data(), &rhs_addr6) != 1)
                return false;
            in4_addr rhs_addr4;
            std::memcpy(&rhs_addr4, &(rhs_addr6.s6_addr[12]), sizeof(in4_addr));
            return lhs_addr4.s_addr == rhs_addr4.s_addr;
        }

        if (_lhs_family == family_type::ipv4_mapped)
        {
            if (_rhs_family == family_type::ipv4)
            {
                in6_addr lhs_addr6;
                if (inet_pton(AF_INET6, _lhs.data(), &lhs_addr6) != 1)
                    return false;
                in4_addr lhs_addr4;
                std::memcpy(&lhs_addr4, &(lhs_addr6.s6_addr[12]), sizeof(in4_addr));
                in4_addr rhs_addr4;
                if (inet_pton(AF_INET, _rhs.data(), &rhs_addr4) != 1)
                    return false;
                return lhs_addr4.s_addr == rhs_addr4.s_addr;
            }
            else if (_rhs_family == family_type::ipv4_mapped)
            {
                in6_addr lhs_addr6;
                if (inet_pton(AF_INET6, _lhs.data(), &lhs_addr6) != 1)
                    return false;
                in6_addr rhs_addr6;
                if (inet_pton(AF_INET6, _rhs.data(), &rhs_addr6) != 1)
                    return false;

                return memcmp(lhs_addr6.s6_addr, rhs_addr6.s6_addr, 16) == 0;
            }
        }

        return _lhs == _rhs;
    }
private:
    address_storage addr_;
    family_type family_;
};

} // namespace mmswpp


#endif // !MMSWPP_INET_ADDR_H_INCLUDED
