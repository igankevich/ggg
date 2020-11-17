#include <chrono>
#include <iostream>

#include <unistdx/fs/idirectory>

#include <ggg/core/database.hh>
#include <ggg/core/native.hh>
#include <ggg/cli/expunge.hh>

namespace {

    inline void
    send(sys::signal s, const std::vector<ggg::Process>& processes) {
        for (const ggg::Process& p : processes) {
            try {
                sys::process_view(p.pid).send(s);
            } catch (const sys::bad_call& err) {
                ggg::native_message(
                    std::cerr,
                    "Failed to send \"_\" signal process _ owned by _: _",
                    s,
                    p.pid,
                    p.uid,
                    err.what()
                );
            }
        }
    }

}

void
ggg::Expunge::parse_arguments(int argc, char* argv[]) {
    Command::parse_arguments(argc, argv);
    if (!this->_args.empty()) {
        throw std::invalid_argument("please, do not specify entity names");
    }
}

void
ggg::Expunge::execute() {
    this->find_expired_entities();
    this->find_processes();
    if (this->_processes.empty()) {
        return;
    }
    for (const Process& p : this->_processes) {
        native_message(
            std::clog,
            "terminating process _ owned by _",
            p.pid,
            p.uid
        );
    }
    send(sys::signal::stop, this->_processes);
    send(sys::signal::terminate, this->_processes);
    send(sys::signal::resume, this->_processes);
}

void
ggg::Expunge::find_expired_entities() {
    this->_uids.clear();
    Database db(Database::File::All);
    auto rstr = db.expired_ids();
    while (rstr.step() != sqlite::errc::done) {
        sqlite::cstream cstr(rstr);
        sys::uid_type id = bad_uid;
        cstr >> id;
        this->_uids.insert(id);
    }
}

void
ggg::Expunge::find_processes() {
    this->_processes.clear();
    sys::path p("/proc");
    sys::idirectory proc(p);
    if (!proc.is_open()) {
        native_message(std::cerr, "unable to open _ directory for reading", p);
        return;
    }
    for (const auto& entry : proc) {
        int pid = std::atoi(entry.name());
        if (pid <= 1) {
            continue;
        }
        sys::path pp(p, entry.name());
        try {
            sys::file_status status(pp);
            if (this->_uids.find(status.owner()) != this->_uids.end()) {
                _processes.emplace_back(pid, status.owner());
            }
        } catch (const sys::bad_call& err) {
            native_message(
                std::cerr,
                "Error getting files status of _: _.",
                pp,
                err.what()
            );
        }
    }
}
