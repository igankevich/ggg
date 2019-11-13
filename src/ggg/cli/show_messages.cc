#include <algorithm>
#include <iostream>

#include <ggg/cli/show_messages.hh>
#include <ggg/config.hh>
#include <ggg/core/database.hh>
#include <ggg/core/native.hh>

void
ggg::Show_messages::execute() {
    auto nargs = this->_args.size();
    using Statement = Database::statement_type;
    Database db(Database::File::Accounts, Database::Flag::Read_only);
    Statement st;
	if (nargs == 0) {
        st = db.messages();
    } else if (nargs == 1) {
        const auto& name = args().front();
        st = db.messages(name.data());
    } else {
        st = db.messages(args().begin(), args().end());
    }
    for (const auto& msg : st.rows<message>()) {
        std::cout << msg << '\n';
    }
    st.close();
    db.close();
}

void
ggg::Show_messages::print_usage() {
	std::cout << "usage: " GGG_EXECUTABLE_NAME " "
		<< this->prefix() << " [ENTITY...]\n";
}
