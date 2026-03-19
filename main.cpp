#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <iostream>
#include <fstream>
#include <set>
#include <string>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <functional>
#include <unistd.h>

namespace fs = std::filesystem;

const char *r = "\033[1;31m";
const char *g = "\033[1;32m";
const char *y = "\033[1;33m";
const char *rr = "\033[0m";
const char *e = "";

const char *green() {
    return isatty(fileno(stdout)) ? g : e;
}

const char *red() {
    return isatty(fileno(stderr)) ? r : e;
}

const char *yellow() {
    return isatty(fileno(stderr)) ? y : e;
}

const char *reset() {
    return isatty(fileno(stderr)) ? rr : e;
}

void endproc(int errc) {
    std::exit(errc);
}

void prefix(const char *text, std::function<const char *()> oops, std::ostream &out) {
    out << '[' << oops() << text << reset() << ']';
}

void prefix(std::string &text, std::function<const char *()> oops, std::ostream &out) {
    prefix(text.c_str(), oops, out);
}

void fatal(int errc, const char *message) {
    prefix("FATAL", red, std::cerr);
    std::cerr << ' ' << message << '\n';
    endproc(errc);
}

void error(const char *message) {
    prefix("ERROR", red, std::cerr);
    std::cerr << ' ' << message << '\n';
}

void warn(const char *message) {
    prefix("WARNING", yellow, std::cerr);
    std::cerr << ' ' << message << '\n';
}

void success(const char *message) {
    prefix("SUCCESS", green, std::cout);
    std::cout << ' ' << message << '\n';
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

void parse_input(int argc, char *argv[], std::string &mode, std::string &compiler, std::string &standard, std::string &output_name, std::set<std::string> &libs, std::set<std::string> &excludes) {
    std::string arg;
    std::string argb;
    std::string argf;
    try {
        for (int i = 1; i < argc; ++i) {
            arg = argv[i];
            if (arg.substr(0, std::min(6UL, arg.size())) == "--help") fatal(-1, "Usage: cbuild [-B(debug | release)] [-C(compiler)] [-S(cxx/c++xx)] [-L(dynamic_libs)...] [-O(executable_name)]");
            argb = arg.substr(0, std::min(arg.size(), 2UL));
            argf = arg.substr(std::min(2UL, arg.size()), arg.size());
            if (argb == "-B") {
                mode = argf;
            } else if (argb == "-C") {
                compiler = argf;
            } else if (argb == "-S") {
                standard += argf;
            } else if (argb == "-L") {
                libs.insert(argf);
            } else if (argb == "-O") {
                output_name = argf;
            } else if (argb == "-X") {
                excludes.insert(argf);
            }
        }
    } catch (std::exception &e) {
        fatal(-1, ("Couldn't pass arg: [" + arg + "] due to: " + e.what()).c_str());
    }
}

int main(int argc, char *argv[]) {
    if (fs::exists(fs::current_path() / "compile_commands.json")) fs::remove(fs::current_path() / "compile_commands.json");
    std::string debug_flags = "-g -Wall -Wextra -fsanitize=address -O0";
    std::string release_flags = "-s -flto -DNDEBUG -O2";
    std::string mode = "debug";
    std::string compiler = "g++";
    std::string standard = "-std=";
    std::string output_name = "exec";
    std::set<std::string> libs = {};
    std::set<std::string> excludes = {};
    parse_input(argc, argv, mode, compiler, standard, output_name, libs, excludes);
    std::transform(mode.begin(), mode.end(), mode.begin(), [](const char c) {
        return std::tolower(c);
    });
    bool imode = 1;
    if (mode == "release") imode = 0;
    fs::remove(fs::current_path() / output_name);
    std::vector<std::string> src_files = {};
    std::set<std::string> header_dirs = {};
    std::set<std::string> lib_dirs = {};
    fs::recursive_directory_iterator it{fs::current_path().c_str()};
    for (const auto& entry : it) {
        auto fname = entry.path().filename().string();
        if (fname.empty() || *fname.begin() == '.' || *fname.begin() == '_' || (fs::is_directory(entry.path()) && excludes.count(fname))) {
            if (entry.is_directory()) it.disable_recursion_pending();
            continue;
        }

        if (entry.is_directory()) continue;
        if (entry.path().filename().string().substr(0, 1) == ".") continue;
        if (entry.path().filename().string().substr(0, 1) == "_") continue;
        std::string ext = entry.path().extension().string();
        switch (valid_ext(ext)) {
            case 0: src_files.push_back(fs::absolute(entry.path())); break;
            case 1: header_dirs.insert(fs::absolute(entry.path().parent_path())); break;
            case 2: lib_dirs.insert(fs::absolute(entry.path().parent_path())); break;
            default: warn(("Found and couldn't parse file: " + fname).c_str()); break;
        }
    }
    std::string final_command = compiler;
    for (const std::string &file : src_files) {
        final_command += ' ' + file;
    }
    for (const std::string &paths : header_dirs) {
        final_command += " -I" + paths;
    }
    for (const std::string &dir : lib_dirs) {
        final_command += " -L" + dir;
    }
    final_command += " -o " + output_name;
    if (imode) {
        final_command += ' ' + debug_flags;
    } else final_command += ' ' + release_flags;
    final_command += ' ' + standard;

    for (const std::string &lib : libs) {
        final_command += " -l" + lib;
    }
    std::cout << "Compiling using following command: \"" << final_command << "\"\n";
    int rez = std::system(final_command.c_str());
    int errc = WEXITSTATUS(rez);
    if (errc) {
        fatal(-1, "Failed Compilation! For more information check output above.");
    } else {
        success("Compilation passed successfully! Output binary has been generated with the name exec.");
        success("Writing `compile_commands.json` file for IDE compatibility!");
        std::ofstream json("compile_commands.json");
        json << "[\n";
        for (size_t i = 0; i < src_files.size(); ++i) {
            json << "  {\n";
            json << "    \"directory\": \"" << fs::current_path().string() << "\",\n";
            // We use the command we built, but language servers prefer the full compiler path
            json << "    \"command\": \"" << compiler << " " << final_command << "\",\n";
            json << "    \"file\": \"" << src_files[i] << "\"\n";
            json << "  }" << (i == src_files.size() - 1 ? "" : ",") << "\n";
        }
        json << "]";
        json.close();
    }
    return 0;
}
