#include <iomanip>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <vector>

#include <unistdx/ipc/identity>

#include <ggg/cli/add_entity.hh>
#include <ggg/cli/cli_traits.hh>
#include <ggg/cli/editor.hh>
#include <ggg/cli/quiet_error.hh>
#include <ggg/cli/tmpfile.hh>
#include <ggg/config.hh>
#include <ggg/core/native.hh>
#include <ggg/guile/guile_traits.hh>
#include <ggg/guile/init.hh>

void
ggg::Add_entity
::parse_arguments(int argc, char* argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "t:f:e:i:")) != -1) {
        switch (opt) {
            case 't': std::string(::optarg) >> this->_type; break;
            case 'f': this->_filename = ::optarg; break;
            case 'e': this->_expression = ::optarg; break;
            case 'i': std::string(::optarg) >> this->_iformat; break;
        }
    }
    for (int i=::optind; i<argc; ++i) {
        this->_args.emplace_back(argv[i]);
    }
    if (this->_args.empty() && this->_filename.empty() && this->_expression.empty()) {
        throw std::invalid_argument("please, specify entity names, filename or Guile expression");
    }
    remove_duplicate_arguments();
}

void
ggg::Add_entity
::execute() {
    guile_init();
    Store store;
    switch (this->_type) {
        case Entity_type::Entity:
            store.open(Store::File::All, Store::Flag::Read_write);
            this->do_execute<entity>(store);
            break;
        case Entity_type::Account:
            store.open(Store::File::All, Store::Flag::Read_write);
            this->do_execute<account>(store);
            break;
        case Entity_type::Public_key:
            store.open(Store::File::Accounts, Store::Flag::Read_write);
            this->do_execute<public_key>(store);
            break;
        case Entity_type::Machine:
            throw std::invalid_argument("not implemented");
        case Entity_type::Message:
            throw std::invalid_argument("adding messages is not allowed");
    }
}

template <class T>
void
ggg::Add_entity
::do_execute(Store& store) {
    if (!this->_filename.empty() || !this->_expression.empty()) {
        if (!this->_filename.empty()) {
            sys::tmpfile tmp;
            tmp.out().imbue(std::locale::classic());
            if (this->_filename == "-") {
                tmp.out() << std::cin.rdbuf();
            } else {
                tmp.out() << file_to_string(this->_filename);
            }
            tmp.out().flush();
            this->insert<T>(store, file_to_string(tmp.filename()));
        }
        if (!this->_expression.empty()) {
            this->insert<T>(store, this->_expression);
        }
    } else {
        this->add_interactive<T>(store);
    }
}

template <class T>
void
ggg::Add_entity
::add_interactive(Store& store) {
    using traits_type = Guile_traits<T>;
    bool success = true;
    /* TODO this check is not universal (does not work for public keys)
    for (const std::string& ent : this->args()) {
        if (store.has(T(ent.data()))) {
            success = false;
            std::stringstream tmp;
            tmp << ent;
            native_message(std::cerr, "Entity _ already exists.", tmp.str());
        }
    }*/
    if (!success) { throw quiet_error(); }
    do {
        sys::tmpfile tmp(".scm");
        tmp.out().imbue(std::locale::classic());
        traits_type::to_guile(tmp.out(), traits_type::generate(args()));
        tmp.out().flush();
        edit_file_or_throw(tmp.filename());
        try {
            this->insert<T>(store, file_to_string(tmp.filename()));
        } catch (const std::exception& err) {
            success = false;
            native_message(std::cerr, "_", err.what());
            native_message(std::cerr, "Press any key to continue...");
            std::cin.get();
        }
    } while (!success);
}

template <class T>
void
ggg::Add_entity
::insert(Store& store, std::string guile) {
    using traits_type = Base_traits<T>;
    using object_array = typename traits_type::object_array;
    // TODO this is inefficient
    std::stringstream tmp(guile);
    object_array objects;
    read<T>(tmp, objects, input_format());
    //auto ents = guile_traits_type::from_guile(guile);
    Transaction tr(store);
    for (const auto& o : objects) { store.insert(o); }
    tr.commit();
}

namespace ggg {

    template <>
    void
    Add_entity
    ::insert<public_key>(Store& store, std::string guile) {
        using traits_type = Base_traits<public_key>;
        using object_array = typename traits_type::object_array;
        // TODO this is inefficient
        std::stringstream tmp(guile);
        object_array objects;
        read<public_key>(tmp, objects, input_format());
        //auto ents = guile_traits_type::from_guile(guile);
        Transaction tr(store);
        for (const auto& o : objects) { store.insert(o); }
        tr.commit();
    }

}

void
ggg::Add_entity
::print_usage() {
    const int w = 20;
    std::cout
        << "usage: " GGG_EXECUTABLE_NAME " "
        << this->prefix() << " [-t TYPE] [-i FORMAT] [-e EXPR] [NAME...]\n"
        << std::setw(w) << std::left << "  -t TYPE"
        << "entity type (account, entity, machine, public-key)\n"
        << std::setw(w) << std::left << "  -i FORMAT"
        << "input format (scm, passwd, group, shadow, ssh)\n"
        << std::setw(w) << std::left << "  -f FILE"
        << "file to read entities from\n";
}
