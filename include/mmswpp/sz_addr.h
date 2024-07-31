
#ifndef MMSWPP_SZ_ADDR_H_INCLUDED
#define MMSWPP_SZ_ADDR_H_INCLUDED

#include <mego/predef/os/linux.h>
#include <mego/predef/os/windows.h>

#include <memepp/string.hpp>
#include <memepp/string_view.hpp>
#include <megopp/util/scope_cleanup.h>

#if MG_OS__WIN_AVAIL
#include <ws2tcpip.h>
#endif

#if MG_OS__LINUX_AVAIL
#include <arpa/inet.h>
#endif

namespace mmswpp {

    enum class sz_addr_e : uint8_t
    {
        none = 0,
        ipv4 = 1,
        ipv6 = 2,
        ipv6_over_ipv4 = 3,
        domain = 4,
    };

    struct sz_addr
    {
        using string = memepp::string;
        using string_view = memepp::string_view;

        sz_addr()
            : data_("0.0.0.0")
            , type_(sz_addr_e::ipv4)
        {
        }

        sz_addr(const sz_addr& _other)
            : data_(_other.data_)
            , type_(_other.type_)
        {
        }

        sz_addr(sz_addr&& _other) noexcept
            : data_(std::move(_other.data_))
            , type_(_other.type_)
        {
        }

        sz_addr(const string_view& _s)
            : data_(_s.trim_space().to_string())
        {
            type_ = identify_type(data_);
        }

        inline sz_addr& operator=(const sz_addr& _other)
        {
            if (this != &_other)
            {
                data_ = _other.data_;
                type_ = _other.type_;
            }
            return *this;
        }

        inline sz_addr& operator=(sz_addr&& _other) noexcept
        {
            if (this != &_other)
            {
                data_ = std::move(_other.data_);
                type_ = _other.type_;
            }
            return *this;
        }

        inline string to_string() const noexcept
        {
            return data_;
        }

        inline sz_addr_e type() const noexcept
        {
            return type_;
        }

        inline constexpr bool is_ip_format() const noexcept
        {
            return  type_ == sz_addr_e::ipv4 || 
                    type_ == sz_addr_e::ipv6 || 
                    type_ == sz_addr_e::ipv6_over_ipv4;
        }

        inline constexpr bool is_domain_format() const noexcept
        {
            return type_ == sz_addr_e::domain;
        }

        inline bool to_sockaddr_storage(uint16_t _port, sockaddr_storage* _out) const noexcept
        {
            if (!is_ip_format())
                return false;

            if (type_ == sz_addr_e::ipv4)
            {
                sockaddr_in* addr = reinterpret_cast<sockaddr_in*>(_out);
                addr->sin_family = AF_INET;
                addr->sin_port  = htons(_port);
                if (inet_pton(AF_INET, data_.c_str(), &addr->sin_addr) != 1)
                    return false;
            }
            else if (type_ == sz_addr_e::ipv6 || type_ == sz_addr_e::ipv6_over_ipv4)
            {
                sockaddr_in6* addr = reinterpret_cast<sockaddr_in6*>(_out);
                addr->sin6_family = AF_INET6;
                addr->sin6_port  = htons(_port);
                if (inet_pton(AF_INET6, data_.c_str(), &addr->sin6_addr) != 1)
                    return false;
            }
            return true;
        }

        template<typename _Container>
        inline void split_domain(std::back_insert_iterator<_Container> _out) const
        {
            if (!is_domain_format())
                return;
            
            data_.split(".", _out);
        }

        inline bool operator==(const sz_addr& _other) const noexcept
        {
            switch (type_) {
            case sz_addr_e::ipv4:
            {
                if (_other.type() == sz_addr_e::ipv6_over_ipv4)
                {
                    return _other.data_.contains(data_);
                }
            } break;
            case sz_addr_e::ipv6_over_ipv4:
            {
                if (_other.type() == sz_addr_e::ipv4)
                {
                    return data_.contains(_other.data_);
                }
            } break;
            default:
                break;
            }

            return data_ == _other.data_;
        }

        inline void set_data(const string_view& _s)
        {
            data_ = _s.trim_space().to_string();
            type_ = identify_type(data_);
        }

        inline bool operator!=(const sz_addr& _other) const noexcept
        {
            return !(*this == _other);
        }

        inline static sz_addr_e identify_type(const string& _s)
        {
            uint8_t buf[INET6_ADDRSTRLEN] = { 0 };
            if (inet_pton(AF_INET, _s.data(), buf) == 1)
            {
                return sz_addr_e::ipv4;
            }
            else if (inet_pton(AF_INET6, _s.data(), buf) == 1)
            {
                if (_s.starts_with("::ffff:") || _s.starts_with("::FFFF:"))
                    return sz_addr_e::ipv6_over_ipv4;
                else
                    return sz_addr_e::ipv6;
            }
            else {
                return sz_addr_e::domain;
            }
        }

    private:
    
        string data_;
        sz_addr_e type_;    
    };

}

#endif // !MMSWPP_SZ_ADDR_H_INCLUDED
