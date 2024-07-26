
#ifndef MMSWPP_IN_ADDR_H_INCLUDED
#define MMSWPP_IN_ADDR_H_INCLUDED

#include <mego/predef/os/linux.h>
#include <mego/util/os/windows/windows_simplify.h>
#include <mego/mem/cstr_alloc_if_no_end_zero.h>

#if MG_OS__LINUX_AVAIL
#  include <netinet/in.h>
#  include <arpa/inet.h>
#endif

#include <memepp/string_def.hpp>
#include <memepp/string_view_def.hpp>
#include <megopp/util/scope_cleanup.h>
#include <string>
#include <string_view>

namespace mmswpp {

struct in04_addr
{
    in04_addr()
    {
        addr_.s_addr = htonl(INADDR_ANY);
    }

    in04_addr(const ::in_addr& _other)
        : addr_(_other)
    {
    }

    in04_addr(const in04_addr& _other)
        : addr_(_other.addr_)
    {
    }

    in04_addr(const char* _str, mmint_t _len)
    {
        const char* str = nullptr;
        mgec_t ec = mgmem__cstr_alloc_if_no_end_zero(_str, _len, &str, NULL, 0);
        if (ec) {
            addr_.s_addr = htonl(INADDR_ANY);
        }
        MEGOPP_UTIL__ON_SCOPE_CLEANUP([&] { 
            mgmem__free_if_ptr_not_equal(_str, (void*)str); });
        
        if (inet_aton(AF_INET, str, &addr_) != 0)
        {
            addr_.s_addr = htonl(INADDR_ANY);
        }
    }

    in04_addr(const char* _str)
        : in04_addr(_str, -1)
    {
    }

    in04_addr(const std::string& _str)
        : in04_addr(_str.c_str(), static_cast<mmint_t>(_str.size()))
    {
    }

    in04_addr(const std::string_view& _str)
        : in04_addr(_str.data(), static_cast<mmint_t>(_str.size()))
    {
    }

    in04_addr(const memepp::string& _str)
        : in04_addr(_str.c_str(), _str.size())
    {
    }

    in04_addr(const memepp::string_view& _str)
        : in04_addr(_str.data(), _str.size())
    {
    }

    template <typename T>
    inline T to() const
    {
        return T{};
    }

    constexpr const ::in_addr& raw() const noexcept
    {
        return addr_;
    }

    constexpr ::in_addr& raw() noexcept
    {
        return addr_;
    }

        template <>
        inline ::in_addr to<::in_addr>() const
        {
            return addr_;
        }

        template <>
        inline ::in6_addr to<::in6_addr>() const
        {
            auto s = to<std::string>();
            ::in6_addr addr;
            if (inet_pton(AF_INET6, s.c_str(), &addr) != 1)
            {
                return ::in6_addr{};
            }

            return addr;
        }

        template <>
        inline std::string to<std::string>() const
        {
            char buffer[INET_ADDRSTRLEN];
            if (inet_ntop(AF_INET, &addr_, buffer, INET_ADDRSTRLEN) == nullptr)
            {
                return std::string{};
            }
            return std::string{ buffer };
        }

        template <>
        inline memepp::string to<memepp::string>() const
        {
            char buffer[INET_ADDRSTRLEN];
            if (inet_ntop(AF_INET, &addr_, buffer, INET_ADDRSTRLEN) == nullptr)
            {
                return memepp::string{};
            }
            return memepp::string{ buffer };
        }

private:
    ::in_addr addr_;
};

struct in06_addr
{
    in06_addr()
    {
        addr_ = in6addr_any;
    }

    in06_addr(const ::in6_addr& _other)
        : addr_(_other)
    {
    }

    in06_addr(const in06_addr& _other)
        : addr_(_other.addr_)
    {
    }

    in06_addr(const char* _str, mmint_t _len)
    {
        const char* str = nullptr;
        mgec_t ec = mgmem__cstr_alloc_if_no_end_zero(_str, _len, &str, NULL, 0);
        if (ec) {
            addr_ = in6addr_any;
        }
        MEGOPP_UTIL__ON_SCOPE_CLEANUP([&] { 
            mgmem__free_if_ptr_not_equal(_str, (void*)str); });
        
        if (inet_pton(AF_INET6, str, &addr_) != 1)
        {
            addr_ = in6addr_any;
        }
    }

    in06_addr(const char* _str)
        : in06_addr(_str, -1)
    {
    }

    in06_addr(const std::string& _str)
        : in06_addr(_str.c_str(), static_cast<mmint_t>(_str.size()))
    {
    }

    in06_addr(const std::string_view& _str)
        : in06_addr(_str.data(), static_cast<mmint_t>(_str.size()))
    {
    }

    in06_addr(const memepp::string& _str)
        : in06_addr(_str.c_str(), _str.size())
    {
    }

    in06_addr(const memepp::string_view& _str)
        : in06_addr(_str.data(), _str.size())
    {
    }

    template <typename T>
    inline T to() const
    {
        return T{};
    }

    constexpr const ::in6_addr& raw() const noexcept
    {
        return addr_;
    }

    constexpr ::in6_addr& raw() noexcept
    {
        return addr_;
    }

    template <>
    inline ::in6_addr to<::in6_addr>() const
    {
        return addr_;
    }

    template <>
    inline std::string to<std::string>() const
    {
        char buffer[INET6_ADDRSTRLEN];
        if (inet_ntop(AF_INET6, &addr_, buffer, INET6_ADDRSTRLEN) == nullptr)
        {
            return std::string{};
        }
        return std::string{ buffer };
    }

    template <>
    inline memepp::string to<memepp::string>() const
    {
        char buffer[INET6_ADDRSTRLEN];
        if (inet_ntop(AF_INET6, &addr_, buffer, INET6_ADDRSTRLEN) == nullptr)
        {
            return memepp::string{};
        }
        return memepp::string{ buffer };
    }

private:
    ::in6_addr addr_;
};

}

#endif // !MMSWPP_IN_ADDR_H_INCLUDED
