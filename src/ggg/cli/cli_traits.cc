#include <ostream>
#include <string>
#include <vector>

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
    void CLI_traits<entity>::write<Entity_output_format::Rec>(
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
    void CLI_traits<entity>::write<Entity_output_format::TSV>(
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
    void CLI_traits<entity>::write<Entity_output_format::NSS>(
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
    void CLI_traits<entity>::write<Entity_output_format::Guile>(
        std::ostream& out,
        statement_type& st
    ) {
        auto rows = st.rows<object_type>();
        Guile_traits<object_type>::to_guile(out, object_array{rows.begin(),rows.end()});
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
    void CLI_traits<account>::write<Entity_output_format::Rec>(
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
    void CLI_traits<account>::write<Entity_output_format::TSV>(
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
    void CLI_traits<account>::write<Entity_output_format::NSS>(
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
    void CLI_traits<account>::write<Entity_output_format::Guile>(
        std::ostream& out,
        statement_type& st
    ) {
        auto rows = st.rows<object_type>();
        Guile_traits<object_type>::to_guile(out, object_array{rows.begin(),rows.end()});
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
    void CLI_traits<message>::write<Entity_output_format::Rec>(
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
    void CLI_traits<message>::write<Entity_output_format::TSV>(
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
    void CLI_traits<message>::write<Entity_output_format::NSS>(
        std::ostream& out,
        statement_type& st
    ) {
        throw std::runtime_error("messages do not have NSS format");
    }

    template <>
    template <>
    void CLI_traits<message>::write<Entity_output_format::Guile>(
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
    void CLI_traits<Machine>::write<Entity_output_format::Rec>(
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
    void CLI_traits<Machine>::write<Entity_output_format::TSV>(
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
    void CLI_traits<Machine>::write<Entity_output_format::NSS>(
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
    void CLI_traits<Machine>::write<Entity_output_format::Guile>(
        std::ostream& out,
        statement_type& st
    ) {
        auto rows = st.rows<object_type>();
        Guile_traits<object_type>::to_guile(out, object_array{rows.begin(),rows.end()});
    }

    template <>
    auto CLI_traits<Machine>::select(
        Database& db,
        const string_array& names
    ) -> statement_type {
        return db.find_machines(names.begin(), names.end());
    }

}
