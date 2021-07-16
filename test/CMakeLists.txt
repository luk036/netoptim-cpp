cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(NetOptimTests LANGUAGES CXX)

# ---- Options ----

option(ENABLE_TEST_COVERAGE "Enable test coverage" OFF)
option(TEST_INSTALLED_VERSION "Test the version found by find_package" OFF)

# --- Import tools ----

include(../cmake/tools.cmake)

# ---- Dependencies ----

include(../cmake/CPM.cmake)

CPMAddPackage("gh:onqtam/doctest#2.4.5")
CPMAddPackage("gh:TheLartians/Format.cmake@1.7.0")

include(../cmake/specific.cmake)

if(TEST_INSTALLED_VERSION)
  find_package(NetOptim REQUIRED)
else()
  CPMAddPackage(NAME NetOptim SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/..)
endif()

# ---- Create binary ----

file(GLOB sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)
add_executable(NetOptimTests ${sources})
target_link_libraries(
  NetOptimTests
  doctest::doctest
  NetOptim::NetOptim
  XNetwork::XNetwork
  Boost::boost
  cppcoro::cppcoro
  Threads::Threads
)
set_target_properties(NetOptimTests PROPERTIES CXX_STANDARD 20)

# enable compiler warnings
if(NOT TEST_INSTALLED_VERSION)
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(NetOptim PUBLIC -Wall -Wpedantic -Wextra -Werror)
  elseif(MSVC)
    target_compile_options(NetOptim PUBLIC /W4 /WX)
    target_compile_definitions(NetOptimTests PUBLIC DOCTEST_CONFIG_USE_STD_HEADERS)
  endif()
endif()

# ---- Add NetOptimTests ----

enable_testing()

# Note: doctest and similar testing frameworks can automatically configure CMake tests. For other
# testing frameworks add the tests target instead: add_test(NAME netoptimTests COMMAND
# NetOptimTests)

include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)
doctest_discover_tests(NetOptimTests)

# ---- code coverage ----

if(ENABLE_TEST_COVERAGE)
  target_compile_options(NetOptim PUBLIC -O0 -g -fprofile-arcs -ftest-coverage)
  target_link_options(NetOptim PUBLIC -fprofile-arcs -ftest-coverage)
endif()
