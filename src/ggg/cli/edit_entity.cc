#include <iostream>
#include <iomanip>
#include <string>
#include <set>
#include <iterator>
#include <vector>
#include <cstdlib>

#include <unistdx/fs/path>
#include <unistdx/io/fdstream>

#include <ggg/cli/edit_entity.hh>
#include <ggg/cli/editor.hh>
#include <ggg/cli/quiet_error.hh>
#include <ggg/cli/tmpfile.hh>
#include <ggg/config.hh>
#include <ggg/core/native.hh>
#include <ggg/guile/guile_traits.hh>
#include <ggg/guile/init.hh>

namespace {

    template <class Container>
    void
    check_duplicates(const Container& objs) {
        const size_t n = objs.size();
        for (size_t i=0; i<n; ++i) {
            const auto& lhs = objs[i];
            for (size_t j=i+1; j<n; ++j) {
                const auto& rhs = objs[j];
                if (lhs == rhs) {
                    throw std::invalid_argument("duplicate names/ids");
                }
            }
        }
    }

}

void
ggg::Edit_entity::parse_arguments(int argc, char* argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "t:f:e:")) != -1) {
        switch (opt) {
            case 't': std::string(::optarg) >> this->_type; break;
            case 'f': this->_filename = ::optarg; break;
            case 'e': this->_expression = ::optarg; break;
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
ggg::Edit_entity::execute()  {
    guile_init();
    Database db;
    switch (this->_type) {
        case Entity_type::Entity:
            db.open(Database::File::Entities, Database::Flag::Read_write);
            this->edit_objects<entity>(db);
            break;
        case Entity_type::Account:
            db.open(Database::File::Accounts, Database::Flag::Read_write);
            this->edit_objects<account>(db);
            break;
        case Entity_type::Public_key:
            db.open(Database::File::Accounts, Database::Flag::Read_write);
            this->edit_objects<public_key>(db);
            break;
        case Entity_type::Machine:
            throw std::invalid_argument("not implemented");
        case Entity_type::Message:
            throw std::invalid_argument("editing messages is not allowed");
    }
}

template <class T>
void
ggg::Edit_entity::edit_objects(Database& db) {
    if (!this->_filename.empty() || !this->_expression.empty()) {
        this->edit_batch<T>(db);
    } else {
        this->edit_interactive<T>(db);
    }
}

template <class T>
void
ggg::Edit_entity::edit_interactive(Database& db) {
    bool success = true;
    do {
        sys::tmpfile tmp(".scm");
        tmp.out().imbue(std::locale::classic());
        this->print_objects<T>(db, tmp.out());
        edit_file_or_throw(tmp.filename());
        // TODO editing account add password field with symbols: 'retain 'remove
        try {
            this->update<T>(db, file_to_string(tmp.filename()));
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
ggg::Edit_entity::edit_batch(Database& db) {
    if (!this->_filename.empty()) {
        sys::tmpfile tmp;
        tmp.out().imbue(std::locale::classic());
        if (this->_filename == "-") {
            tmp.out() << std::cin.rdbuf();
        } else {
            tmp.out() << file_to_string(this->_filename);
        }
        tmp.out().flush();
        this->update<T>(db, file_to_string(tmp.filename()));
    }
    if (!this->_expression.empty()) {
        this->update<T>(db, this->_expression);
    }
}

template <class T>
void
ggg::Edit_entity::print_objects(Database& db, std::ostream& out) {
    using traits_type = Guile_traits<T>;
    std::vector<T> entities;
    for (const auto& name : this->args()) {
        const auto& ent = traits_type::select(name.data());
        if (!ent.empty()) { entities.insert(entities.end(), ent.begin(), ent.end()); }
    }
    if (entities.empty()) { throw std::runtime_error("not found"); }
    traits_type::to_guile(out, entities);
    out.flush();
}

void
ggg::Edit_entity::print_usage() {
    const int w = 20;
    std::cout << "usage: " GGG_EXECUTABLE_NAME " "
        << this->prefix() << " [-t TYPE] [-f FILE] [-e EXPR] [NAME...]\n"
        << std::setw(w) << std::left << "  -t TYPE"
        << "entity type (account, entity, machine, public-key)\n"
        << std::setw(w) << std::left << "  -f FILE"
        << "file to read objects from\n"
        << std::setw(w) << std::left << "  -e EXPR"
        << "read objects by evaluating Guile expression\n";
}

template <class T>
void
ggg::Edit_entity::update(Database& db, const std::string& guile) {
    using guile_traits_type = Guile_traits<T>;
    auto ents = guile_traits_type::from_guile(guile);
    check_duplicates(ents);
    Transaction tr(db);
    for (const auto& ent : ents) { db.update(ent); }
    tr.commit();
}

template void ggg::Edit_entity::print_objects<ggg::entity>(Database& db, std::ostream& out);
template void ggg::Edit_entity::print_objects<ggg::account>(Database& db, std::ostream& out);
template void ggg::Edit_entity::update<ggg::entity>(Database&, const std::string&);
template void ggg::Edit_entity::update<ggg::account>(Database&, const std::string&);
template void ggg::Edit_entity::edit_objects<ggg::entity>(Database& db);
template void ggg::Edit_entity::edit_objects<ggg::account>(Database& db);
template void ggg::Edit_entity::edit_batch<ggg::entity>(Database& db);
template void ggg::Edit_entity::edit_batch<ggg::account>(Database& db);
template void ggg::Edit_entity::edit_interactive<ggg::entity>(Database& db);
template void ggg::Edit_entity::edit_interactive<ggg::account>(Database& db);
