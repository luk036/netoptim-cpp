#include <netoptim/version.h>  // for NETOPTIM_VERSION

#include <cxxopts.hpp>    // for value, OptionAdder, Options, OptionValue
#include <iostream>       // for string, operator<<, endl, basic_ostream
#include <memory>         // for shared_ptr
#include <string>         // for char_traits, hash, operator==
#include <unordered_map>  // for operator==, unordered_map, __hash_map_...

auto main(int argc, char **argv) -> int {
    cxxopts::Options options(*argv, "A program to welcome the world!");

    std::string language;
    std::string name;

    // clang-format off
  options.add_options()
    ("h,help", "Show help")
    ("vtx,version", "Print the current version number")
    ("n,name", "Name to greet", cxxopts::value(name)->default_value("World"))
    ("l,lang", "Language code to use", cxxopts::value(language)->default_value("en"))
  ;
    // clang-format on

    auto result = options.parse(argc, argv);

    if (result["help"].as<bool>()) {
        std::cout << options.help() << std::endl;
        return 0;
    }

    if (result["version"].as<bool>()) {
        std::cout << "NetOptim, version " << NETOPTIM_VERSION << std::endl;
        return 0;
    }

    return 0;
}
