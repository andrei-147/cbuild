#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <set>
#include <string>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

void endproc(const char *message) {
    std::cerr << message << '\n';
    std::exit(-1);
}

int valid_ext(std::string &ext) {
    if (ext == ".cpp") return 0;
    if (ext == ".c") return 0;
    if (ext == ".h") return 1;
    if (ext == ".hpp") return 1;
    if (ext == ".lib") return 2;
    if (ext.substr(0, 3) == ".so") return 2;
    return -1;
}

int main(int argc, char *argv[]) {
    if (argc < 2) endproc("[ERROR] Usage: make++ {debug | release} [compiler]");
    std::string mode = argv[1];
    std::string compiler = "g++";
    if (argc == 3) compiler = argv[2];
    std::transform(mode.begin(), mode.end(), mode.begin(), [](const char c) {
        return std::tolower(c);
    });
    bool imode = 0;
    if (std::strcmp(mode.c_str(), "debug") == 0) {
        imode = 1;
    }
    std::cout << "Compiling in [" << (imode ? "DEBUG" : "RELEASE") << "] using the " << compiler << " compiler.\n";
    std::string output_name = "exec";
    fs::remove(fs::current_path() / output_name);
    std::vector<std::string> src_files = {};
    std::set<std::string> header_dirs = {};
    fs::recursive_directory_iterator it{fs::current_path().c_str()};
    for (const auto& entry : it) {
        if (entry.is_directory()) continue;
        if (entry.path().filename().string().substr(0, 1) == "_") continue;
        std::string ext = entry.path().extension().string();
        switch (valid_ext(ext)) {
            case 0: src_files.push_back(fs::absolute(entry.path())); break;
            case 1: header_dirs.insert(fs::absolute(entry.path().parent_path())); break;
            case 2: std::cout << "[SORRY] Ezcompile can't properly link library files at the moment!\n";
            default: std::cerr << "[WARNING] Couldn't figure out what to do with file " << entry.path().filename().string() + entry.path().extension().string() << "!\n"; break;
        }
    }
    std::string final_command = compiler;
    for (const std::string &file : src_files) {
        final_command += ' ' + file;
    }
    for (const std::string &paths : header_dirs) {
        final_command += " -I" + paths;
    }
    final_command += " -o " + output_name;
    int rez = std::system(final_command.c_str());
    int errc = WEXITSTATUS(rez);
    if (errc) {
        std::cerr << "[ERROR] Failed compilation! Please check output.\n";
    } else {
        std::cout << "[SUCCESS] Compilation passed successfully! Output binary has been generated with the name exec.\n";
    }
    return 0;
}
