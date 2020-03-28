#include <unistd.h>
#include <iostream>
#include <stdexcept>

#include <ggg/cli/cli_traits.hh>
#include <ggg/config.hh>
#include <ggg/core/database.hh>
#include <ggg/core/native.hh>

int main(int argc, char* argv[]) {
    using namespace ggg;
    int ret = EXIT_FAILURE;
    if (argc < 2) {
        native_message(std::cerr, "please, specify name as an argument");
        return ret;
    }
    if (argc > 2) {
        native_message(std::cerr, "please, specify exactly one argument");
        return ret;
    }
    #if !defined(GGG_DEVELOPER_BUILD)
    try {
    #endif
        const char* name = argv[1];
        Database db(Database::File::Accounts, Database::Flag::Read_only);
        auto st = db.public_keys(name);
        write<public_key>(std::cout, st, Format::SSH);
        st.close();
        db.close();
        ret = EXIT_SUCCESS;
    #if !defined(GGG_DEVELOPER_BUILD)
    } catch (const std::bad_alloc& err) {
        native_message(std::cerr, "memory allocation error");
    } catch (const std::exception& err) {
        error_message(std::cerr, err);
    }
    #endif
    return ret;
}
