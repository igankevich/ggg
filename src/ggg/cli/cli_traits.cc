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
    void CLI_traits<entity,Format::Rec>::write(
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
    void CLI_traits<entity,Format::TSV>::write(
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
    void CLI_traits<entity,Format::Passwd>::write(
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
    void CLI_traits<entity,Format::Passwd>::read(
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
    void CLI_traits<entity,Format::Group>::write(
        std::ostream& out,
        statement_type& st
    ) {
        auto conn = st.connection();
        std::string name;
        for (const auto& ent : st.rows<object_type>()) {
            out << ent.name() << ":x:";
            out << ent.id() << ':';
            auto members = ggg::Connection(conn).find_users_by_group_name(ent.name().data());
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
    void CLI_traits<entity,Format::SCM>::write(
        std::ostream& out,
        statement_type& st
    ) {
        auto rows = st.rows<object_type>();
        Guile_traits<object_type>::to_guile(out, object_array{rows.begin(),rows.end()});
    }

    template <>
    void CLI_traits<entity,Format::SCM>::read(
        std::istream& in,
        object_array& result
    ) {
        std::stringstream tmp;
        tmp << in.rdbuf();
        result = Guile_traits<object_type>::from_guile(tmp.str());
    }

    template <>
    auto Base_traits<entity>::select(
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
    void CLI_traits<account,Format::Rec>::write(
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
    void CLI_traits<account,Format::TSV>::write(
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
    void CLI_traits<account,Format::Shadow>::write(
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
    void CLI_traits<account,Format::Shadow>::read(
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
    void CLI_traits<account,Format::SCM>::write(
        std::ostream& out,
        statement_type& st
    ) {
        auto rows = st.rows<object_type>();
        Guile_traits<object_type>::to_guile(out, object_array{rows.begin(),rows.end()});
    }

    template <>
    void CLI_traits<account,Format::SCM>::read(
        std::istream& in,
        object_array& result
    ) {
        std::stringstream tmp;
        tmp << in.rdbuf();
        result = Guile_traits<object_type>::from_guile(tmp.str());
    }

    template <>
    auto Base_traits<account>::select(
        Database& db,
        const string_array& names
    ) -> statement_type {
        return db.find_accounts(names.begin(), names.end());
    }

    template <>
    void CLI_traits<message,Format::Rec>::write(
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
    void CLI_traits<message,Format::TSV>::write(
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
    void CLI_traits<message,Format::SCM>::write(
        std::ostream& out,
        statement_type& st
    ) {
        auto rows = st.rows<object_type>();
        Guile_traits<object_type>::to_guile(out, object_array{rows.begin(),rows.end()});
    }

    template <>
    auto Base_traits<message>::select(
        Database& db,
        const string_array& names
    ) -> statement_type {
        return db.messages(names.begin(), names.end());
    }

    template <>
    void CLI_traits<Machine,Format::Rec>::write(
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
    void CLI_traits<Machine,Format::TSV>::write(
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
    void CLI_traits<Machine,Format::Hosts>::write(
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
    void CLI_traits<Machine,Format::SCM>::write(
        std::ostream& out,
        statement_type& st
    ) {
        auto rows = st.rows<object_type>();
        Guile_traits<object_type>::to_guile(out, object_array{rows.begin(),rows.end()});
    }

    template <>
    void CLI_traits<Machine,Format::SCM>::read(
        std::istream& in,
        object_array& result
    ) {
        std::stringstream tmp;
        tmp << in.rdbuf();
        result = Guile_traits<object_type>::from_guile(tmp.str());
    }

    template <>
    auto Base_traits<Machine>::select(
        Database& db,
        const string_array& names
    ) -> statement_type {
        return db.find_machines(names.begin(), names.end());
    }

    template <>
    void CLI_traits<public_key,Format::Rec>::write(
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
    void CLI_traits<public_key,Format::TSV>::write(
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
    void CLI_traits<public_key,Format::SSH>::write(
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
    void CLI_traits<public_key,Format::SCM>::write(
        std::ostream& out,
        statement_type& st
    ) {
        auto rows = st.rows<object_type>();
        Guile_traits<object_type>::to_guile(out, object_array{rows.begin(),rows.end()});
    }

    template <>
    void CLI_traits<public_key,Format::SCM>::read(
        std::istream& in,
        object_array& result
    ) {
        std::stringstream tmp;
        tmp << in.rdbuf();
        result = Guile_traits<object_type>::from_guile(tmp.str());
    }

    template <>
    void CLI_traits<public_key,Format::SSH>::read(
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
    auto Base_traits<public_key>::select(
        Database& db,
        const string_array& names
    ) -> statement_type {
        return db.public_keys(names.begin(), names.end());
    }

}

template <> void
ggg::write<ggg::entity>(std::ostream& out, sqlite::statement& st, Format format) {
    using T = entity;
    switch (format) {
        case Format::Rec: CLI_traits<T,Format::Rec>::write(out, st); break;
        case Format::SCM: CLI_traits<T,Format::SCM>::write(out, st); break;
        case Format::TSV: CLI_traits<T,Format::TSV>::write(out, st); break;
        case Format::Passwd: CLI_traits<T,Format::Passwd>::write(out, st); break;
        case Format::Name:
            for (const auto& obj : st.template rows<T>()) {
                out << obj.name() << '\n';
            }
            break;
        default: throw std::invalid_argument("format is not supported");
    }
}

template <> void
ggg::write<ggg::account>(std::ostream& out, sqlite::statement& st, Format format) {
    using T = account;
    switch (format) {
        case Format::Rec: CLI_traits<T,Format::Rec>::write(out, st); break;
        case Format::SCM: CLI_traits<T,Format::SCM>::write(out, st); break;
        case Format::TSV: CLI_traits<T,Format::TSV>::write(out, st); break;
        case Format::Shadow: CLI_traits<T,Format::Shadow>::write(out, st); break;
        case Format::Name:
            for (const auto& obj : st.template rows<T>()) {
                out << obj.name() << '\n';
            }
            break;
        default: throw std::invalid_argument("format is not supported");
    }
}

template <> void
ggg::write<ggg::public_key>(std::ostream& out, sqlite::statement& st, Format format) {
    using T = public_key;
    switch (format) {
        case Format::Rec: CLI_traits<T,Format::Rec>::write(out, st); break;
        case Format::SCM: CLI_traits<T,Format::SCM>::write(out, st); break;
        case Format::TSV: CLI_traits<T,Format::TSV>::write(out, st); break;
        case Format::SSH: CLI_traits<T,Format::SSH>::write(out, st); break;
        case Format::Name:
            for (const auto& obj : st.template rows<T>()) {
                out << obj.name() << '\n';
            }
            break;
        default: throw std::invalid_argument("format is not supported");
    }
}

template <> void
ggg::write<ggg::Machine>(std::ostream& out, sqlite::statement& st, Format format) {
    using T = Machine;
    switch (format) {
        case Format::Rec: CLI_traits<T,Format::Rec>::write(out, st); break;
        case Format::SCM: CLI_traits<T,Format::SCM>::write(out, st); break;
        case Format::TSV: CLI_traits<T,Format::TSV>::write(out, st); break;
        case Format::Hosts: CLI_traits<T,Format::Hosts>::write(out, st); break;
        case Format::Name:
            for (const auto& obj : st.template rows<T>()) {
                out << obj.name() << '\n';
            }
            break;
        default: throw std::invalid_argument("format is not supported");
    }
}


void
ggg::read(std::istream& in, std::vector<public_key>& result, Format format) {
    using T = public_key;
    switch (format) {
        case Format::SCM: CLI_traits<T,Format::SCM>::read(in, result); break;
        case Format::SSH: CLI_traits<T,Format::SSH>::read(in, result); break;
        default: throw std::invalid_argument("format is not supported");
    }
}
