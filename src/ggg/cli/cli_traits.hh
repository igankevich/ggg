#ifndef GGG_CLI_CLI_TRAITS_HH
#define GGG_CLI_CLI_TRAITS_HH

#include <ostream>
#include <string>
#include <vector>

#include <sqlitex/statement.hh>

#include <ggg/cli/entity_type.hh>
#include <ggg/core/account.hh>
#include <ggg/core/entity.hh>
#include <ggg/core/machine.hh>
#include <ggg/core/public_key.hh>

namespace ggg {

    class Database;

    template <class T>
    struct Base_traits {

        using object_type = T;
        using object_array = std::vector<T>;
        using string_array = std::vector<std::string>;
        using statement_type = sqlite::statement;

        static auto select(Database& db, const string_array& names) -> statement_type;

    };

    template <class T, Format format>
    struct CLI_traits: public Base_traits<T> {

        using typename Base_traits<T>::object_type;
        using typename Base_traits<T>::object_array;
        using typename Base_traits<T>::string_array;
        using typename Base_traits<T>::statement_type;

        static void write(std::ostream& out, statement_type& st);
        static void read(std::istream& in, object_array& result);

    };

    template <class T>
    inline void
    write(std::ostream& out, sqlite::statement& st, Format format) {
        switch (format) {
            case Format::Rec: CLI_traits<T,Format::Rec>::write(out, st); break;
            case Format::SCM: CLI_traits<T,Format::SCM>::write(out, st); break;
            case Format::TSV: CLI_traits<T,Format::TSV>::write(out, st); break;
            case Format::Name:
                for (const auto& obj : st.template rows<T>()) {
                    out << obj.name() << '\n';
                }
                break;
            default:
                throw std::invalid_argument("format is not supported");
        }
    }

    template <> void
    write<entity>(std::ostream& out, sqlite::statement& st, Format format);

    template <> void
    write<account>(std::ostream& out, sqlite::statement& st, Format format);

    template <> void
    write<public_key>(std::ostream& out, sqlite::statement& st, Format format);

    template <> void
    write<Machine>(std::ostream& out, sqlite::statement& st, Format format);

    template <class T>
    inline void
    read(std::istream& in, std::vector<T>& result, Format format) {
        switch (format) {
            case Format::SCM: CLI_traits<T,Format::SCM>::read(in, result); break;
            default: throw std::invalid_argument("format is not supported");
        }
    }

    void
    read(std::istream& in, std::vector<public_key>& result, Format format);

}

#endif // vim:filetype=cpp
