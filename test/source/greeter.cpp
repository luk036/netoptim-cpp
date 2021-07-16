#include <doctest/doctest.h>
#include <netoptim/greeter.h>
#include <netoptim/version.h>

#include <string>

TEST_CASE("NetOptim") {
    using namespace netoptim;

    NetOptim netoptim("Tests");

    CHECK(netoptim.greet(LanguageCode::EN) == "Hello, Tests!");
    CHECK(netoptim.greet(LanguageCode::DE) == "Hallo Tests!");
    CHECK(netoptim.greet(LanguageCode::ES) == "¡Hola Tests!");
    CHECK(netoptim.greet(LanguageCode::FR) == "Bonjour Tests!");
}

TEST_CASE("NetOptim version") {
    static_assert(std::string_view(NETOPTIM_VERSION) == std::string_view("1.0"));
    CHECK(std::string(NETOPTIM_VERSION) == std::string("1.0"));
}
