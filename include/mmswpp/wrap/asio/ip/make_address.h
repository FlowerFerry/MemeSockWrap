
#ifndef MMSWPP_WRAP_ASIO_IP_MAKE_ADDRESS_H_INCLUDED
#define MMSWPP_WRAP_ASIO_IP_MAKE_ADDRESS_H_INCLUDED

#include <asio/ip/address.hpp>
#include <memepp/string.hpp>
#include <memepp/string_view.hpp>

namespace mmswpp {
namespace wrap {
namespace asio {
namespace ip {

inline ::asio::ip::address_v6 make_address_v6(const memepp::string& _str, ::asio::error_code& _ec)
{
    auto addr4 = ::asio::ip::make_address_v4(_str.data(), _ec);
    if (!_ec) {
        return ::asio::ip::make_address_v6(::asio::ip::v4_mapped, addr4);
    }
    return ::asio::ip::make_address_v6(_str.data(), _ec);
}

inline ::asio::ip::address_v6 make_address_v6(const memepp::string& _str)
{
    ::asio::error_code ec;
    auto addr = make_address_v6(_str, ec);
    if (ec) {
        throw ::asio::system_error(ec);
    }
    return addr;
}

inline ::asio::ip::address_v6 make_address_v6(const memepp::string_view& _str, ::asio::error_code& _ec)
{
    return make_address_v6(_str.to_string(), _ec);
}

inline ::asio::ip::address_v6 make_address_v6(const memepp::string_view& _str)
{
    return make_address_v6(_str.to_string());
}

inline ::asio::ip::address_v4 make_address_v4(const memepp::string& _str, ::asio::error_code& _ec)
{  
    auto addr6 = asio::ip::make_address_v6(_str, _ec);
    if (!_ec) {
        if (addr6.is_v4_mapped()) {
            return ::asio::ip::make_address_v4(::asio::ip::v4_mapped, addr6);
        }
        else
            return {};
    }

    return ::asio::ip::make_address_v4(_str.data(), _ec);
}

inline ::asio::ip::address_v4 make_address_v4(const memepp::string& _str)
{
    ::asio::error_code ec;
    auto addr = make_address_v4(_str, ec);
    if (ec) {
        throw ::asio::system_error(ec);
    }
    return addr;
}

inline ::asio::ip::address_v4 make_address_v4(const memepp::string_view& _str, ::asio::error_code& _ec)
{
    return make_address_v4(_str.to_string(), _ec);
}

inline ::asio::ip::address_v4 make_address_v4(const memepp::string_view& _str)
{
    return make_address_v4(_str.to_string());
}

} // namespace mmswpp::wrap::asio::ip
} // namespace mmswpp::wrap::asio
} // namespace mmswpp::wrap
} // namespace mmswpp

#endif // !MMSWPP_WRAP_ASIO_IP_MAKE_ADDRESS_H_INCLUDED
