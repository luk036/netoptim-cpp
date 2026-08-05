#include <netoptim/version.h>

#include <iostream>

auto main() -> int {
    const auto ok = (NETOPTIM_VERSION_MAJOR >= 1);
    std::cout << "netoptim installed test: version " << NETOPTIM_VERSION << "\n";
    return ok ? 0 : 1;
}
