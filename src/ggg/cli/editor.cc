#include "editor.hh"

#include <cstdlib>
#include <stdexcept>
#include <unistdx/io/pipe>
#include <unistdx/io/fdstream>
#include <unistdx/ipc/process>
#include <unistdx/ipc/execute>
#include <unistdx/fs/idirtree>

#include <ggg/config.hh>

std::string
ggg::find_file_editor() {
    std::string result;
    if (const char* editor = std::getenv("EDITOR")) {
        result = editor;
    } else {
        result = GGG_FILE_EDITOR;
    }
    return result;
}

sys::process_status
ggg::edit_file(std::string path) {
    std::string editor = find_file_editor();
    sys::process child([&editor,&path] () {
        sys::argstream args;
        args.append(editor);
        args.append(path);
        return sys::this_process::execute_command(args);
    });
    return child.wait();
}

void
ggg::edit_file_or_throw(std::string path) {
    sys::process_status status = ::ggg::edit_file(path);
    if (!(status.exited() && status.exit_code() == 0)) {
        throw std::runtime_error("bad exit code from editor");
    }
}
