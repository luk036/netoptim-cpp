cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(NetOptimBenchmarks LANGUAGES CXX)

# ---- Options ----

option(ENABLE_TEST_COVERAGE "Enable test coverage" OFF)

# ---- Dependencies ----

include(../cmake/CPM.cmake)
include(../cmake/tools.cmake)
include(../specific.cmake)

CPMAddPackage("gh:TheLartians/Format.cmake@1.7.3")

# Add the library target if it doesn't already exist
if(NOT TARGET NetOptim)
  CPMAddPackage(NAME NetOptim SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/..)
endif()

# ---- bench_neg_cycle ----

add_executable(bench_neg_cycle source/bench_neg_cycle.cpp)
target_link_libraries(bench_neg_cycle NetOptim::NetOptim ${SPECIFIC_LIBS})
target_include_directories(bench_neg_cycle PRIVATE ../include)
set_target_properties(bench_neg_cycle PROPERTIES CXX_STANDARD 20)

# ---- bench_network_oracle ----

add_executable(bench_network_oracle source/bench_network_oracle.cpp)
target_link_libraries(
  bench_network_oracle NetOptim::NetOptim ${SPECIFIC_LIBS} ${SPECIFIC_ABSEIL_LIBS}
)
target_include_directories(bench_network_oracle PRIVATE ../include)
set_target_properties(bench_network_oracle PROPERTIES CXX_STANDARD 20)

# enable compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  target_compile_options(bench_neg_cycle PRIVATE -Wall -Wpedantic -Wextra -Werror)
  target_compile_options(bench_network_oracle PRIVATE -Wall -Wpedantic -Wextra -Werror)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  target_compile_options(bench_neg_cycle PRIVATE -Wall -Wpedantic -Wextra -Werror)
  target_compile_options(bench_network_oracle PRIVATE -Wall -Wpedantic -Wextra -Werror)
elseif(MSVC)
  target_compile_options(bench_neg_cycle PRIVATE /utf-8 /W4 /WX /wd4702)
  target_compile_options(bench_network_oracle PRIVATE /utf-8 /W4 /WX /wd4702)
endif()
