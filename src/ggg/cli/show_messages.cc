#include <algorithm>
#include <iostream>

#include <ggg/cli/show_messages.hh>
#include <ggg/config.hh>
#include <ggg/core/database.hh>
#include <ggg/core/native.hh>

void
ggg::Show_messages::execute() {
    auto nargs = this->_args.size();
    Database db(Database::File::Accounts, Database::Flag::Read_only);
	if (nargs == 0) {
        for (const auto& msg : db.messages().rows<message>()) {
            auto t = message::clock_type::to_time_t(msg.timestamp());
            char buf[1024];
            std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S%z", std::localtime(&t));
            std::cout << buf << "  " << msg.hostname() << "  "
                << msg.name() << "  " << msg.text() << "  " << '\n';
        }
    } else if (nargs == 1) {
        const auto& name = args().front();
        for (const auto& msg : db.messages(name.data()).rows<message>()) {
            auto t = message::clock_type::to_time_t(msg.timestamp());
            char buf[1024];
            std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S%z", std::localtime(&t));
            std::cout << buf << "  " << msg.hostname() << "  "
                << msg.text() << "  " << '\n';
        }
    }
    db.close();
}

void
ggg::Show_messages::print_usage() {
	std::cout << "usage: " GGG_EXECUTABLE_NAME " "
		<< this->prefix() << " [ENTITY]\n";
}


