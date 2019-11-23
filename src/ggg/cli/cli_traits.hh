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

        template <Entity_output_format format>
        static void write(std::ostream& out, statement_type& st);

        static auto select(Database& db, const string_array& names) -> statement_type;

    };

    template <class T>
    inline void
    write(
        std::ostream& out,
        typename CLI_traits<T>::statement_type& st,
        Entity_output_format format
    ) {
        switch (format) {
            case Entity_output_format::Rec:
                CLI_traits<T>::template write<Entity_output_format::Rec>(out, st);
                break;
            case Entity_output_format::Guile:
                CLI_traits<T>::template write<Entity_output_format::Guile>(out, st);
                break;
            case Entity_output_format::TSV:
                CLI_traits<T>::template write<Entity_output_format::TSV>(out, st);
                break;
            case Entity_output_format::NSS:
                CLI_traits<T>::template write<Entity_output_format::NSS>(out, st);
                break;
            case Entity_output_format::Name:
                for (const auto& obj : st.template rows<T>()) {
                    out << obj.name() << '\n';
                }
                break;
        }
    }

}

#endif // vim:filetype=cpp
