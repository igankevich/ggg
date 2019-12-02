#ifndef GGG_CLI_CLI_TRAITS_HH
#define GGG_CLI_CLI_TRAITS_HH

#include <ostream>
#include <string>
#include <vector>

#include <sqlitex/statement.hh>

#include <ggg/cli/entity_type.hh>

namespace ggg {

    class Database;

    template <class T>
    struct CLI_traits {

        using object_type = T;
        using object_array = std::vector<T>;
        using string_array = std::vector<std::string>;
        using statement_type = sqlite::statement;

        template <Format format>
        static void write(std::ostream& out, statement_type& st);

        template <Format format>
        inline static void read(std::istream& in, object_array& result) {
            throw std::invalid_argument("not supported");
        }

        static auto select(Database& db, const string_array& names) -> statement_type;

    };

    template <class T>
    inline void
    write(
        std::ostream& out,
        typename CLI_traits<T>::statement_type& st,
        Format format
    ) {
        switch (format) {
            case Format::Rec:
                CLI_traits<T>::template write<Format::Rec>(out, st);
                break;
            case Format::SCM:
                CLI_traits<T>::template write<Format::SCM>(out, st);
                break;
            case Format::TSV:
                CLI_traits<T>::template write<Format::TSV>(out, st);
                break;
            case Format::NSS:
                CLI_traits<T>::template write<Format::NSS>(out, st);
                break;
            case Format::SSH:
                CLI_traits<T>::template write<Format::SSH>(out, st);
                break;
            case Format::Name:
                for (const auto& obj : st.template rows<T>()) {
                    out << obj.name() << '\n';
                }
                break;
        }
    }

    template <class T>
    inline void
    read(
        std::istream& in,
        typename CLI_traits<T>::object_array& result,
        Format format
    ) {
        switch (format) {
            case Format::SCM:
                CLI_traits<T>::template read<Format::SCM>(in, result);
                break;
            case Format::NSS:
                CLI_traits<T>::template read<Format::NSS>(in, result);
                break;
            case Format::SSH:
                CLI_traits<T>::template read<Format::SSH>(in, result);
                break;
            default:
                throw std::invalid_argument("not supported");
        }
    }

}

#endif // vim:filetype=cpp
