#include <iomanip>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>

#include <ggg/core/account.hh>
#include <ggg/core/database.hh>
#include <ggg/core/days.hh>
#include <ggg/core/entity.hh>
#include <ggg/core/machine.hh>
#include <ggg/core/message.hh>

#include <ggg/cli/cli_traits.hh>
#include <ggg/guile/guile_traits.hh>

namespace {

    void
    put_time_and_date(std::ostream& out, ggg::account::time_point tp) {
        if (tp > ggg::account::time_point(ggg::account::duration::zero())) {
            auto t = ggg::account::clock_type::to_time_t(tp);
            char buf[25];
            std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S%z", std::localtime(&t));
            out << buf;
        } else {
            out << '-';
        }
    }

    void
    put_status(
        std::ostream& out,
        const ggg::account& acc,
        ggg::account::time_point now
    ) {
        std::vector<std::string> status;
        if (acc.has_expired(now)) { status.push_back("EXPIRED"); }
        if (acc.password_has_expired(now)) { status.push_back("PASSWORD_EXPIRED"); }
        if (acc.has_been_suspended()) { status.push_back("SUSPENDED"); }
        if (status.empty()) { status.push_back("OK"); }
        std::copy(
            status.begin(),
            status.end(),
            sys::intersperse_iterator<std::string,char>(out, '|')
        );
    }

}


namespace ggg {

    template <>
    template <>
    void CLI_traits<entity>::write<Format::Rec>(
        std::ostream& out,
        statement_type& st
    ) {
        for (const auto& ent : st.rows<object_type>()) {
            out << "name: " << ent.name() << '\n';
            out << "id: " << ent.id() << '\n';
            out << "description: " << ent.description() << '\n';
            out << "home: " << ent.home() << '\n';
            out << "shell: " << ent.shell() << '\n';
            out << '\n';
        }
    }

    template <>
    template <>
    void CLI_traits<entity>::write<Format::TSV>(
        std::ostream& out,
        statement_type& st
    ) {
        for (const auto& ent : st.rows<object_type>()) {
            out << ent.name() << '\t';
            out << ent.id() << '\t';
            out << ent.description() << '\t';
            out << ent.home() << '\t';
            out << ent.shell();
            out << '\n';
        }
    }

    template <>
    template <>
    void CLI_traits<entity>::write<Format::Passwd>(
        std::ostream& out,
        statement_type& st
    ) {
        for (const auto& ent : st.rows<object_type>()) {
            out << ent.name() << ":x:";
            out << ent.id() << ':';
            out << ent.id() << ':';
            out << ent.description() << ':';
            out << ent.home() << ':';
            out << ent.shell();
            out << '\n';
        }
    }

    template <>
    template <>
    void CLI_traits<entity>::read<Format::Passwd>(
        std::istream& in,
        object_array& result
    ) {
        std::string line;
        int line_no = 0;
        while (in >> std::ws && std::getline(in, line)) {
            ++line_no;
            // remove trailing spaces
            while (!line.empty() && std::isspace(line.back())) { line.pop_back(); }
            std::istringstream tmp(line);
            entity o;
            if (!(tmp >> o)) {
                std::ostringstream msg;
                msg << "bad entity at line " << line_no << ": " << line;
                throw std::invalid_argument(msg.str());
            }
            result.emplace_back(std::move(o));
        }
    }

    template <>
    template <>
    void CLI_traits<entity>::write<Format::Group>(
        std::ostream& out,
        statement_type& st
    ) {
        auto conn = st.connection();
        std::string name;
        for (const auto& ent : st.rows<object_type>()) {
            out << ent.name() << ":x:";
            out << ent.id() << ':';
            auto members = children(conn, ent.id());
            if (members.step() != sqlite::errc::done) {
                members.column(0, name);
                out << name;
            }
            while (members.step() != sqlite::errc::done) {
                members.column(0, name);
                out << ',' << name;
            }
            out << '\n';
        }
    }

    template <>
    template <>
    void CLI_traits<entity>::write<Format::SCM>(
        std::ostream& out,
        statement_type& st
    ) {
        auto rows = st.rows<object_type>();
        Guile_traits<object_type>::to_guile(out, object_array{rows.begin(),rows.end()});
    }

    template <>
    template <>
    void CLI_traits<entity>::read<Format::SCM>(
        std::istream& in,
        object_array& result
    ) {
        std::stringstream tmp;
        tmp << in.rdbuf();
        result = Guile_traits<object_type>::from_guile(tmp.str());
    }

    template <>
    auto CLI_traits<entity>::select(
        Database& db,
        const string_array& names
    ) -> statement_type {
        auto st = db.find_non_existing_entities(names.begin(), names.end());
        std::string msg, name;
        while (st.step() != sqlite::errc::done) {
            st.column(0, name);
            msg += name;
            msg += " not found\n";
        }
        if (!msg.empty()) { msg.pop_back(); throw std::invalid_argument(msg); }
        return db.find_entities(names.begin(), names.end());
    }

    template <>
    template <>
    void CLI_traits<account>::write<Format::Rec>(
        std::ostream& out,
        statement_type& st
    ) {
        auto now = account::clock_type::now();
        for (const auto& acc : st.rows<object_type>()) {
            out << "name: " << acc.name() << '\n';
            out << "expiration-date: ";
            put_time_and_date(out, acc.expiration_date());
            out << '\n';
            out << "last-change: ";
            put_time_and_date(out, acc.last_change());
            out << '\n';
            out << "status: ";
            put_status(out, acc, now);
            out << '\n';
            out << '\n';
        }
    }

    template <>
    template <>
    void CLI_traits<account>::write<Format::TSV>(
        std::ostream& out,
        statement_type& st
    ) {
        auto now = account::clock_type::now();
        for (const auto& acc : st.rows<object_type>()) {
            out << acc.name() << '\t';
            put_time_and_date(out, acc.expiration_date());
            out << '\t';
            put_time_and_date(out, acc.last_change());
            out << '\t';
            put_status(out, acc, now);
            out << '\n';
        }
    }

    template <>
    template <>
    void CLI_traits<account>::write<Format::Shadow>(
        std::ostream& out,
        statement_type& st
    ) {
        for (const auto& acc : st.rows<object_type>()) {
            out << acc.name() << ":!::::::";
            if (acc.has_expiration_date()) {
                out << to_days(acc.expiration_date());
            }
            out << ':';
            out << '\n';
        }
    }

    template <>
    template <>
    void CLI_traits<account>::read<Format::Shadow>(
        std::istream& in,
        object_array& result
    ) {
        std::string line;
        int line_no = 0;
        while (in >> std::ws && std::getline(in, line)) {
            ++line_no;
            // remove trailing spaces
            while (!line.empty() && std::isspace(line.back())) { line.pop_back(); }
            std::istringstream tmp(line);
            account o;
            if (!(tmp >> o)) {
                std::ostringstream msg;
                msg << "bad account at line " << line_no << ": " << line;
                throw std::invalid_argument(msg.str());
            }
            result.emplace_back(std::move(o));
        }
    }

    template <>
    template <>
    void CLI_traits<account>::write<Format::SCM>(
        std::ostream& out,
        statement_type& st
    ) {
        auto rows = st.rows<object_type>();
        Guile_traits<object_type>::to_guile(out, object_array{rows.begin(),rows.end()});
    }

    template <>
    template <>
    void CLI_traits<account>::read<Format::SCM>(
        std::istream& in,
        object_array& result
    ) {
        std::stringstream tmp;
        tmp << in.rdbuf();
        result = Guile_traits<object_type>::from_guile(tmp.str());
    }

    template <>
    auto CLI_traits<account>::select(
        Database& db,
        const string_array& names
    ) -> statement_type {
        return db.find_accounts(names.begin(), names.end());
    }

    template <>
    template <>
    void CLI_traits<message>::write<Format::Rec>(
        std::ostream& out,
        statement_type& st
    ) {
        for (const auto& m : st.rows<object_type>()) {
            out << "name: " << m.name() << '\n';
            out << "hostname: " << m.hostname() << '\n';
            out << "timestamp: ";
            put_time_and_date(out, m.timestamp());
            out << '\n';
            out << "text: " << m.text() << '\n';
            out << '\n';
        }
    }

    template <>
    template <>
    void CLI_traits<message>::write<Format::TSV>(
        std::ostream& out,
        statement_type& st
    ) {
        for (const auto& m : st.rows<object_type>()) {
            put_time_and_date(out, m.timestamp());
            out << '\t';
            out << m.name() << '\t';
            out << m.hostname() << '\t';
            out << m.text();
            out << '\n';
        }
    }

    template <>
    template <>
    void CLI_traits<message>::write<Format::SCM>(
        std::ostream& out,
        statement_type& st
    ) {
        auto rows = st.rows<object_type>();
        Guile_traits<object_type>::to_guile(out, object_array{rows.begin(),rows.end()});
    }

    template <>
    auto CLI_traits<message>::select(
        Database& db,
        const string_array& names
    ) -> statement_type {
        return db.messages(names.begin(), names.end());
    }

    template <>
    template <>
    void CLI_traits<Machine>::write<Format::Rec>(
        std::ostream& out,
        statement_type& st
    ) {
        for (const auto& m : st.rows<object_type>()) {
            out << "name: " << m.name() << '\n';
            out << "ip-address: " << m.address() << '\n';
            out << "ethernet-address: " << m.ethernet_address() << '\n';
            out << '\n';
        }
    }

    template <>
    template <>
    void CLI_traits<Machine>::write<Format::TSV>(
        std::ostream& out,
        statement_type& st
    ) {
        for (const auto& m : st.rows<object_type>()) {
            out << m.name() << '\t';
            out << m.address() << '\t';
            out << m.ethernet_address();
            out << '\n';
        }
    }

    template <>
    template <>
    void CLI_traits<Machine>::write<Format::Hosts>(
        std::ostream& out,
        statement_type& st
    ) {
        for (const auto& m : st.rows<object_type>()) {
            out << m.address() << '\t';
            out << m.name();
            out << '\n';
        }
    }

    template <>
    template <>
    void CLI_traits<Machine>::write<Format::SCM>(
        std::ostream& out,
        statement_type& st
    ) {
        auto rows = st.rows<object_type>();
        Guile_traits<object_type>::to_guile(out, object_array{rows.begin(),rows.end()});
    }

    template <>
    template <>
    void CLI_traits<Machine>::read<Format::SCM>(
        std::istream& in,
        object_array& result
    ) {
        std::stringstream tmp;
        tmp << in.rdbuf();
        result = Guile_traits<object_type>::from_guile(tmp.str());
    }

    template <>
    auto CLI_traits<Machine>::select(
        Database& db,
        const string_array& names
    ) -> statement_type {
        return db.find_machines(names.begin(), names.end());
    }

    template <>
    template <>
    void CLI_traits<public_key>::write<Format::Rec>(
        std::ostream& out,
        statement_type& st
    ) {
        for (const auto& o : st.rows<object_type>()) {
            out << "name: " << o.name() << '\n';
            out << "options: " << o.options() << '\n';
            out << "type: " << o.type() << '\n';
            out << "key: " << o.key() << '\n';
            out << "comment: " << o.comment() << '\n';
            out << '\n';
        }
    }

    template <>
    template <>
    void CLI_traits<public_key>::write<Format::TSV>(
        std::ostream& out,
        statement_type& st
    ) {
        for (const auto& o : st.rows<object_type>()) {
            out << o.name() << '\t';
            out << o.options() << '\t';
            out << o.type() << '\t';
            out << o.key() << '\t';
            out << o.comment();
            out << '\n';
        }
    }

    template <>
    template <>
    void CLI_traits<public_key>::write<Format::SSH>(
        std::ostream& out,
        statement_type& st
    ) {
        for (const auto& o : st.rows<object_type>()) {
            if (!o.options().empty()) { out << o.options() << ' '; }
            out << o.type() << ' ';
            out << o.key() << ' ';
            if (!o.comment().empty()) { out << o.comment(); }
            else { out << o.name() << "-key"; }
            out << '\n';
        }
    }

    template <>
    template <>
    void CLI_traits<public_key>::write<Format::SCM>(
        std::ostream& out,
        statement_type& st
    ) {
        auto rows = st.rows<object_type>();
        Guile_traits<object_type>::to_guile(out, object_array{rows.begin(),rows.end()});
    }

    template <>
    template <>
    void CLI_traits<public_key>::read<Format::SCM>(
        std::istream& in,
        object_array& result
    ) {
        std::stringstream tmp;
        tmp << in.rdbuf();
        result = Guile_traits<object_type>::from_guile(tmp.str());
    }

    template <>
    template <>
    void CLI_traits<public_key>::read<Format::SSH>(
        std::istream& in,
        object_array& result
    ) {
        std::string line;
        int line_no = 0;
        while (in >> std::ws && std::getline(in, line)) {
            ++line_no;
            // remove trailing spaces
            while (!line.empty() && std::isspace(line.back())) { line.pop_back(); }
            std::istringstream tmp(line);
            char ch = 0;
            bool inside_quotes = false;
            std::vector<std::string> values;
            std::string value;
            while (tmp.get(ch)) {
                value += ch;
                if (ch == '"') { inside_quotes = !inside_quotes; }
                else if (!inside_quotes && std::isspace(ch)) {
                    value.pop_back();
                    in >> std::ws;
                    values.emplace_back(std::move(value));
                    value.clear();
                }
            }
            if (!value.empty()) { values.emplace_back(std::move(value)); }
            public_key o;
            switch (values.size()) {
                case 2:
                    o.type(values[0]);
                    o.key(values[1]);
                    break;
                case 3:
                    o.type(values[0]);
                    o.key(values[1]);
                    o.comment(values[2]);
                    break;
                case 4:
                    o.options(values[0]);
                    o.type(values[1]);
                    o.key(values[2]);
                    o.comment(values[3]);
                    break;
                default:
                    std::ostringstream msg;
                    msg << "bad key at line " << line_no << ": " << line;
                    throw std::invalid_argument(msg.str());
            }
            result.emplace_back(std::move(o));
        }
    }

    template <>
    auto CLI_traits<public_key>::select(
        Database& db,
        const string_array& names
    ) -> statement_type {
        return db.public_keys(names.begin(), names.end());
    }

}
