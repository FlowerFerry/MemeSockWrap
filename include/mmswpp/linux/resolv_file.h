
#ifndef MMSWPP_LINUX_RESOLV_FILE_H_INCLUDED
#define MMSWPP_LINUX_RESOLV_FILE_H_INCLUDED

#include <mmswpp/sz_addr.h>

#include <stdio.h>
#include <mego/err/ec.h>
#include <mego/err/ec_impl.h>
#include <mego/predef/os/linux.h>
#include <mego/util/std/file.h>

#if MG_OS__LINUX_AVAIL
#   include <unistd.h>
#   include <fcntl.h>
#endif

#include <map>
#include <tuple>
#include <vector>
#include <memory>
#include <optional>
#include <algorithm>

#include <memepp/variable_buffer.hpp>
#include <memepp/string_view.hpp>
#include <megopp/util/scope_cleanup.h>

namespace mmswpp {
namespace linux  {
namespace resolv {

    struct parameter
    {
        enum class e_type
        {
            unknown,
            domain,
            search,
            options,
            sortlist,
            nameserver,
            comment,
            blankline
        };
        
        virtual ~parameter() = default;

        virtual inline e_type type() const noexcept = 0;

        bool has_own_affixes = false;
    };

    struct unknown_parameter : public parameter
    {
        inline e_type type() const noexcept { return e_type::unknown; }

        memepp::string data;
    };

    struct blankline_parameter : public parameter
    {
        inline e_type type() const noexcept { return e_type::blankline; }
    };

    struct options_parameter : public parameter
    {
        inline e_type type() const noexcept { return e_type::options; }

        memepp::string into_line() const;

        static std::unique_ptr<options_parameter> from_line(memepp::string_view _sv);

        std::optional<int>  timeout;
        std::optional<int>  attempts;
        std::optional<int>  ndots;
        std::optional<bool> rotate;
        std::optional<bool> no_check_names;
        std::optional<bool> inet6;
        std::optional<bool> ip6_bytestring;
        std::optional<int>  ip6_dotint;
        std::optional<bool> edns0;
        std::optional<bool> single_request;
        std::optional<bool> single_request_reopen;
        std::optional<bool> no_tld_query;
        std::optional<bool> use_vc;
        std::optional<bool> no_sigchld;
        std::optional<bool> trust_ad;

        std::vector<memepp::string> unknown;
    };

    struct sortlist_parameter : public parameter
    {
        inline e_type type() const noexcept { return e_type::sortlist; }

        std::vector<memepp::string> list;
    };

    struct domain_parameter : public parameter
    {
        inline e_type type() const noexcept { return e_type::domain; }

        memepp::string data;
    };

    struct search_parameter : public parameter
    {
        inline e_type type() const noexcept { return e_type::search; }

        std::vector<memepp::string> list;
    };

    struct nameserver_parameter : public parameter
    {
        inline e_type type() const noexcept { return e_type::nameserver; }

        sz_addr data;
    };

    struct comment_parameter : public parameter
    {
        inline e_type type() const noexcept { return e_type::comment; }

        memepp::string data;
    };

    struct conf_file
    {
        
        mgrc_t merge_at_end(const conf_file& _other);

        mgrc_t own_clear();

        mgec_t into_file(FILE* _file);
        mgec_t into_file(memepp::string_view _path);

        static std::tuple<std::unique_ptr<conf_file>, mgec_t> from_file(FILE* _file);
        static std::tuple<std::unique_ptr<conf_file>, mgec_t> from_file(memepp::string_view _path);

        static std::tuple<std::unique_ptr<conf_file>, mgec_t> 
            from_file(FILE* _file, const memepp::string_view& _own_prefix, const memepp::string_view& _own_suffix);
        static std::tuple<std::unique_ptr<conf_file>, mgec_t> 
            from_file(memepp::string_view _path, const memepp::string_view& _own_prefix, const memepp::string_view& _own_suffix);

        std::vector<std::unique_ptr<parameter>> parameters;
        memepp::string own_prefix;
        memepp::string own_suffix;
    };

    inline memepp::string options_parameter::into_line() const
    {
        memepp::variable_buffer buf;

        buf.append(memepp::string_view{"options "});

        if (timeout.has_value())
            buf.append(memepp::string_view{"timeout:"}).
                append(memepp::c_format(64, "%d", timeout.value())).
                append(memepp::string_view{" "});
        if (attempts.has_value())
            buf.append(memepp::string_view{"attempts:"}).
                append(memepp::c_format(64, "%d", attempts.value())).
                append(memepp::string_view{" "});
        if (ndots.has_value())
            buf.append(memepp::string_view{"ndots:"}).
                append(memepp::c_format(64, "%d", ndots.value())).
                append(memepp::string_view{" "});
        if (rotate.has_value())
            buf.append(memepp::string_view{"rotate "});
        if (no_check_names.has_value())
            buf.append(memepp::string_view{"no-check-names "});
        if (inet6.has_value())
            buf.append(memepp::string_view{"inet6 "});
        if (ip6_bytestring.has_value())
            buf.append(memepp::string_view{"ip6-bytestring "});
        if (ip6_dotint.has_value())
            buf.append(memepp::string_view{ip6_dotint.value() ? "ip6-dotint " : "no-ip6-dotint "});
        if (edns0.has_value())
            buf.append(memepp::string_view{"edns0 "});
        if (single_request.has_value())
            buf.append(memepp::string_view{"single-request "});
        if (single_request_reopen.has_value())
            buf.append(memepp::string_view{"single-request-reopen "});
        if (no_tld_query.has_value())
            buf.append(memepp::string_view{"no-tld-query "});
        if (use_vc.has_value())
            buf.append(memepp::string_view{"use-vc "});
        if (no_sigchld.has_value())
            buf.append(memepp::string_view{"no-sigchld "});
        if (trust_ad.has_value())
            buf.append(memepp::string_view{"trust-ad "});

        for (auto& u : unknown)
            buf.append(u).append(memepp::string_view{" "});

        memepp::string s;
        buf.release(s);
        return s;
    }

    inline std::unique_ptr<options_parameter> options_parameter::from_line(memepp::string_view _sv)
    {
        std::vector<memepp::string_view> parts;
        _sv.split(" ", memepp::split_behavior_t::skip_empty_parts, std::back_inserter(parts));

        auto p = std::make_unique<options_parameter>();

        for (auto& part : parts)
        {
            if (part.starts_with("timeout:")) 
            {
                auto s = part.substr(8).to_string();
                p->timeout = atoi(s.data());
            }
            else if (part.starts_with("attempts:")) 
            {
                auto s = part.substr(9).to_string();
                p->attempts = atoi(s.data());
            }
            else if (part.starts_with("ndots:")) 
            {
                auto s = part.substr(6).to_string();
                p->ndots = atoi(s.data());
            }
            else if (part == "rotate")
                p->rotate = true;
            else if (part == "no-check-names") 
                p->no_check_names = true;
            else if (part == "inet6")
                p->inet6 = true;
            else if (part == "ip6-bytestring") 
                p->ip6_bytestring = true;
            else if (part == "ip6-dotint")
                p->ip6_dotint = 1;
            else if (part == "no-ip6-dotint") 
                p->ip6_dotint = 0;
            else if (part == "edns0")
                p->edns0 = true;
            else if (part == "single-request") 
                p->single_request = true;
            else if (part == "single-request-reopen") 
                p->single_request_reopen = true;
            else if (part == "no-tld-query") 
                p->no_tld_query = true;
            else if (part == "use-vc")
                p->use_vc = true;
            else if (part == "no-sigchld") 
                p->no_sigchld = true;
            else if (part == "trust-ad") 
                p->trust_ad = true;
            else {
                p->unknown.push_back(part.to_string());
            }
        }

        return p;
    }
    
    // TO_DO
    inline mgrc_t conf_file::merge_at_end(const conf_file& _other)
    {
        mgrc_t result = 0;
        std::map<parameter::e_type, std::vector<parameter*>> src_params;
        for (auto& p : _other.parameters)
            src_params[p->type()].push_back(p.get());

        std::vector<parameter*> append_params;
        auto it = src_params.find(parameter::e_type::nameserver);
        if (it != src_params.end())
        {
            for (auto& p : it->second)
            {
                auto& addr = static_cast<nameserver_parameter*>(p)->data;
                auto it2 = std::find_if(parameters.begin(), parameters.end(), [&](auto& p2) 
                {
                    if (p2->type() != parameter::e_type::nameserver)
                        return false;
                    auto& addr2 = static_cast<nameserver_parameter*>(p2.get())->data;
                    return addr == addr2;
                });
                if (it2 == parameters.end())
                    append_params.push_back(p);
            }

            for (auto& p : append_params)
                parameters.push_back(std::make_unique<nameserver_parameter>(static_cast<nameserver_parameter&>(*p)));
            
            result = append_params.size();
        }

        return result;
    }

    
    inline mgrc_t conf_file::own_clear()
    {
        mgrc_t result = 0;
        for (auto it = parameters.begin(); it != parameters.end();)
        {
            if ((*it)->has_own_affixes)
            {
                it = parameters.erase(it);
                ++result;
            }
            else
                ++it;
        }

        return result;
    }

    inline mgec_t conf_file::into_file(FILE* _file)
    {
        bool affixes_not_empty = !own_prefix.empty() && !own_suffix.empty();

        for (auto& p : parameters)
        {
            if (p->has_own_affixes && affixes_not_empty)
                fprintf(_file, "##%s\n", own_prefix.data());

            switch (p->type()) {
            case parameter::e_type::unknown:
                fprintf(_file, "%s\n", static_cast<unknown_parameter*>(p.get())->data.data());
                break;
            case parameter::e_type::blankline:
                fprintf(_file, "\n");
                break;
            case parameter::e_type::comment:
                fprintf(_file, "%s\n", static_cast<comment_parameter*>(p.get())->data.data());
                break;
            case parameter::e_type::domain:
                fprintf(_file, "domain %s\n", static_cast<domain_parameter*>(p.get())->data.data());
                break;
            case parameter::e_type::search:
            {
                auto& list = static_cast<search_parameter*>(p.get())->list;
                fprintf(_file, "search");
                for (auto& s : list)
                    fprintf(_file, " %s", s.data());
                fprintf(_file, "\n");
                break;
            }
            case parameter::e_type::nameserver:
            {
                auto& addr = static_cast<nameserver_parameter*>(p.get())->data;
                fprintf(_file, "nameserver %s\n", addr.to_string().data());
                break;
            }
            case parameter::e_type::sortlist:
            {
                auto& list = static_cast<sortlist_parameter*>(p.get())->list;
                fprintf(_file, "sortlist");
                for (auto& s : list)
                    fprintf(_file, " %s", s.data());
                fprintf(_file, "\n");
                break;
            }
            case parameter::e_type::options:
            {
                auto opt = static_cast<options_parameter*>(p.get());
                fprintf(_file, "%s\n", opt->into_line().data());

                break;
            }
            default: {
                return MGEC__INVAL;
            }
            }

            if (p->has_own_affixes && affixes_not_empty)
                fprintf(_file, "##%s\n", own_suffix.data());

        }

        return 0;
    }

    inline mgec_t conf_file::into_file(memepp::string_view _path)
    {
        auto fp = mgu_fopen(_path.data(), _path.size(), "w", 1);
        if (!fp)
            return mgec__from_posix_err(errno);
        MEGOPP_UTIL__ON_SCOPE_CLEANUP([&] { mgu_fclose(fp, true); });

        auto ec = into_file(fp);
        if (ec)
            return ec;

        return 0;
    }

    inline std::tuple<std::unique_ptr<conf_file>, mgec_t> 
        conf_file::from_file(FILE* _file, const memepp::string_view& _own_prefix, const memepp::string_view& _own_suffix)
    {
        memepp::variable_buffer buf;
        char line[512];
        std::unique_ptr<conf_file> cfg = std::make_unique<conf_file>();
        cfg->own_prefix = _own_prefix.trim_space().to_string();
        cfg->own_suffix = _own_suffix.trim_space().to_string();

        bool in_affixes_scope  = false;
        bool affixes_not_empty = !cfg->own_prefix.empty() && !cfg->own_suffix.empty();

        while (fgets(line, sizeof(line), _file) != nullptr)
        {
            memepp::string_view line_view{ line };
            if (!line_view.ends_with("\n")) 
            {
                buf.append(line_view);
                continue;
            }

            memepp::string str;
            if (!buf.empty()) {
                buf.append(line_view);

                buf.release(str);
                line_view = str;
            }
            
            line_view = line_view.trim_space();

            if (line_view.empty()) {
                cfg->parameters.push_back(std::make_unique<blankline_parameter>(blankline_parameter{}));
                continue;
            }

            bool is_affixes = false;
            do {
                if (!affixes_not_empty)
                    break;
                if (!line_view.starts_with("##"))
                    break;
                
                if (line_view.contains(cfg->own_prefix)) 
                {
                    in_affixes_scope = true;
                    is_affixes = true;
                    break;
                }
                if (line_view.contains(cfg->own_suffix))
                {
                    in_affixes_scope = false;
                    is_affixes = true;
                    break;
                }
            } while (0);

            if (is_affixes)
                continue;

            if (line_view.at(0) == '#') {
                auto p  = std::make_unique<comment_parameter>();
                p->data = memepp::string{ line };
                cfg->parameters.push_back(std::move(p));
                continue;
            }

            memepp::string_view type;
            memepp::string_view data;
            auto pos = line_view.find(' ');
            if (pos == memepp::string_view::npos) {
                type = line_view;
            } else {
                type = line_view.substr(0, pos );
                data = line_view.substr(pos + 1);
            }

            if (type == "options")
            {
                auto p = options_parameter::from_line(data);
                p->has_own_affixes = in_affixes_scope;

                if (p == nullptr) {
                    return std::make_tuple(nullptr, MGEC__INVAL);
                }
                cfg->parameters.push_back(std::move(p));
            }
            else if (type == "sortlist")
            {
                auto p = std::make_unique<sortlist_parameter>();
                p->has_own_affixes = in_affixes_scope;

                data.split(" ", memepp::split_behavior_t::skip_empty_parts, std::back_inserter(p->list));
                cfg->parameters.push_back(std::move(p));
            }
            else if (type == "domain")
            {
                auto p  = std::make_unique<domain_parameter>();
                p->has_own_affixes = in_affixes_scope;
                p->data = data.to_string();
                cfg->parameters.push_back(std::move(p));
            }
            else if (type == "search")
            {
                auto p = std::make_unique<search_parameter>();
                data.split(" ", memepp::split_behavior_t::skip_empty_parts, std::back_inserter(p->list));
                cfg->parameters.push_back(std::move(p));
            }
            else if (type == "nameserver")
            {
                auto p = std::make_unique<nameserver_parameter>();
                p->has_own_affixes = in_affixes_scope;
                p->data = sz_addr{ data };
                cfg->parameters.push_back(std::move(p));
            }
            else {
                auto p  = std::make_unique<unknown_parameter>();
                p->data = memepp::string{ line };
                cfg->parameters.push_back(std::move(p));
            }
        }

        return std::make_tuple(std::move(cfg), 0);
    }

    inline std::tuple<std::unique_ptr<conf_file>, mgec_t> conf_file::from_file(memepp::string_view _path)
    {
        auto fp = mgu_fopen(_path.data(), _path.size(), "r", 1);
        if (!fp)
            return std::make_tuple(nullptr, mgec__from_posix_err(errno));
        MEGOPP_UTIL__ON_SCOPE_CLEANUP([&] { fclose(fp); });

        return from_file(fp, {}, {});
    }

    inline std::tuple<std::unique_ptr<conf_file>, mgec_t> 
        conf_file::from_file(memepp::string_view _path, const memepp::string_view& _own_prefix, const memepp::string_view& _own_suffix)
    {
        auto fp = mgu_fopen(_path.data(), _path.size(), "r", 1);
        if (!fp)
            return std::make_tuple(nullptr, mgec__from_posix_err(errno));
        MEGOPP_UTIL__ON_SCOPE_CLEANUP([&] { fclose(fp); });

        return from_file(fp, _own_prefix, _own_suffix);
    }

}
}
} // namespace mmswpp

#endif // !MMSWPP_LINUX_RESOLV_FILE_H_INCLUDED
