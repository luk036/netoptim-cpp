cmake_minimum_required(VERSION 3.14...3.22)

project(
  NetOptim
  VERSION 1.2.6
  LANGUAGES CXX
)

# ---- Include guards ----

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(
    FATAL_ERROR
      "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
  )
endif()

# ---------------------------------------------------------------------------
# Compiler warnings (as errors)
# ---------------------------------------------------------------------------

function(target_enable_warnings TARGET)
  if(MSVC)
    target_compile_options(${TARGET} PRIVATE /W4 /WX /utf-8 /wd4702 /wd4100)
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    target_compile_options(${TARGET} PRIVATE -Wall -Wextra -Wpedantic -Werror)
  endif()
endfunction()

# Abseil headers emit warnings that -Werror escalates to errors. Suppress on targets that include
# abseil headers. GCC coroutine frames also mis-link at -O2 (pre-existing test issue), so the
# tests build at -O0.
function(target_enable_abseil_suppressions TARGET)
  if(MSVC)
    target_compile_options(${TARGET} PRIVATE /wd4100 /wd4324)
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    target_compile_options(${TARGET} PRIVATE -Wno-overflow -Wno-pedantic -O0)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang")
      target_compile_options(${TARGET} PRIVATE -Wno-nullability-extension -Wno-gcc-compat)
    endif()
  endif()
endfunction()

# ---------------------------------------------------------------------------
# Options
# ---------------------------------------------------------------------------
option(NETOPTIM_BUILD_TESTS "Build tests" ON)
option(NETOPTIM_BUILD_BENCHMARKS "Build benchmarks" OFF)
option(NETOPTIM_BUILD_STANDALONE "Build standalone executable" ON)
option(NETOPTIM_BUILD_DOCS "Build Doxygen documentation" OFF)
option(NETOPTIM_ENABLE_CLANG_TIDY "Enable clang-tidy analysis" OFF)
option(NETOPTIM_ENABLE_COVERAGE "Enable coverage reporting" OFF)

# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------

# Download CPM.cmake for reproducible dependency management
# see https://github.com/cpm-cmake/CPM.cmake for more info
set(CPM_DOWNLOAD_VERSION 0.38.5)
set(CPM_HASH_SUM "192aa0ccdc57dfe75bd9e4b176bf7fb5692fd2b3e3f7b09c74856fc39572b31c")
set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
if(NOT (EXISTS ${CPM_DOWNLOAD_LOCATION}))
  message(STATUS "Downloading CPM.cmake to ${CPM_DOWNLOAD_LOCATION}")
  file(
    DOWNLOAD https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake
             ${CPM_DOWNLOAD_LOCATION}
    EXPECTED_HASH SHA256=${CPM_HASH_SUM}
  )
endif()
include(${CPM_DOWNLOAD_LOCATION})

# Always build dependencies from source. EllAlgo declares spdlog itself via FetchContent and assumes
# the source-built spdlog targets; the system-package opt-in (CPM_USE_LOCAL_PACKAGES) would swap in an
# imported spdlog::spdlog and break that assumption, so spdlog needs no special handling here.
set(CPM_USE_LOCAL_PACKAGES OFF)

# PackageProject.cmake will be used to make our target installable
CPMAddPackage("gh:TheLartians/PackageProject.cmake@1.8.0")

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# fmt is a transitive dependency of the graph libraries; prefer a system-installed fmt when
# available.
find_package(fmt CONFIG QUIET)
if(fmt_FOUND)
  message(STATUS "Found system fmt: ${fmt_DIR}")
  set(CPM_fmt_ADDED YES)
else()
  CPMAddPackage(
    NAME fmt
    GIT_TAG 12.1.0
    GITHUB_REPOSITORY fmtlib/fmt
    OPTIONS "FMT_INSTALL YES" # create an installable target
  )
endif()

CPMAddPackage(
  NAME Py2Cpp
  GIT_TAG v1.6.4
  GITHUB_REPOSITORY luk036/py2cpp
  OPTIONS "INSTALL_ONLY ON" "PY2CPP_BUILD_TESTS OFF"
)

# Suppress MSVC STL1011 error on <experimental/coroutine> used by xnetwork via /await
if(MSVC)
  add_compile_definitions(_SILENCE_EXPERIMENTAL_COROUTINE_DEPRECATION_WARNINGS)
endif()

CPMAddPackage(
  NAME XNetwork
  GIT_TAG v1.7.7
  GITHUB_REPOSITORY luk036/xnetwork-cpp
  OPTIONS "INSTALL_ONLY ON" "XNETWORK_BUILD_TESTS OFF"
)

# Patch xnetwork testcases.hpp for MSVC v14.51+ strict anonymous enum type traits (error C2139:
# 'create_test_case1::nodes': an undefined class is not allowed as an argument to compiler intrinsic
# type trait '__is_constructible')
if(MSVC AND xnetwork_SOURCE_DIR)
  set(_tc_file "${xnetwork_SOURCE_DIR}/include/xnetwork/generators/testcases.hpp")
  if(EXISTS "${_tc_file}")
    file(READ "${_tc_file}" _tc_content)
    string(REPLACE "enum nodes { A, B, C, D, E };"
                   "constexpr uint32_t A = 0, B = 1, C = 2, D = 3, E = 4;" _tc_content
                   "${_tc_content}"
    )
    string(REPLACE "enum nodes { A, B, C };" "constexpr uint32_t A = 0, B = 1, C = 2;" _tc_content
                   "${_tc_content}"
    )
    file(WRITE "${_tc_file}" "${_tc_content}")
    message(STATUS "Patched xnetwork testcases.hpp for MSVC C2139 compatibility")
    unset(_tc_file)
    unset(_tc_content)
  endif()
endif()

# spdlog is used by the dependencies' logger sources. SPDLOG_FMT_EXTERNAL is always set so spdlog uses
# the external fmt and avoids duplicate symbols.
CPMAddPackage(
  NAME spdlog
  GIT_TAG v1.17.0
  GITHUB_REPOSITORY gabime/spdlog
  OPTIONS "SPDLOG_INSTALL YES" "SPDLOG_FMT_EXTERNAL YES"
)

CPMAddPackage(
  NAME DiGraphX
  GIT_TAG v1.1.7
  GITHUB_REPOSITORY luk036/digraphx-cpp
  OPTIONS "INSTALL_ONLY ON" "DIGRAPHX_BUILD_TESTS OFF"
)

CPMAddPackage(
  NAME EllAlgo
  GIT_TAG v1.6.9
  GITHUB_REPOSITORY luk036/ellalgo-cpp
  OPTIONS "INSTALL_ONLY YES" "ELLALGO_BUILD_TESTS OFF"
)

# DiGraphX (fetched above) already adds abseil via CPM - use its REAL targets if present instead of
# mixing with find_package IMPORTED targets (IMPORTED vs ALIAS conflict).
if(TARGET absl::flat_hash_map)
  message(STATUS "Using CPM-provided abseil (from DiGraphX)")
  set(SPECIFIC_ABSEIL_LIBS absl::flat_hash_map)
else()
  # Try system-installed abseil first (Ubuntu: libabsl-dev, macOS: brew install abseil, Termux: pkg
  # install abseil-cpp). Falls back to CPM build on Windows or when no system package is available.
  find_package(absl CONFIG QUIET)
  if(absl_FOUND)
    message(STATUS "Found system abseil: ${absl_DIR}")
    set(SPECIFIC_ABSEIL_LIBS absl::flat_hash_map)
  else()
    CPMAddPackage(
      NAME abseil-cpp
      GIT_TAG 20260107.1
      GITHUB_REPOSITORY abseil/abseil-cpp
      OPTIONS "ABSL_PROPAGATE_CXX_STD ON"
    )
    set(SPECIFIC_ABSEIL_LIBS absl::flat_hash_map)
  endif()
endif()

set(SPECIFIC_LIBS
    EllAlgo::EllAlgo DiGraphX::DiGraphX XNetwork::XNetwork Py2Cpp::Py2Cpp Threads::Threads fmt::fmt
)

# ellalgo v1.6.9+ compiles spdlog headers in its library but does not link fmt. With
# SPDLOG_FMT_EXTERNAL (set on the shared spdlog above), its logger needs fmt's include dir.
if(TARGET EllAlgo)
  get_target_property(FMT_INCLUDE_DIRS fmt::fmt INTERFACE_INCLUDE_DIRECTORIES)
  target_include_directories(EllAlgo PRIVATE ${FMT_INCLUDE_DIRS})
endif()

# ---------------------------------------------------------------------------
# Library target (header-only)
# ---------------------------------------------------------------------------

# Note: globbing sources is considered bad practice as CMake's generators may not detect new files
# automatically. Keep that in mind when changing files, or explicitly mention them here.
file(GLOB_RECURSE headers CONFIGURE_DEPENDS
     "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
     "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
)

add_library(${PROJECT_NAME} INTERFACE)

# being a cross-platform target, we enforce standards conformance on MSVC
target_compile_options(
  ${PROJECT_NAME} INTERFACE "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/permissive->"
                            "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/wd4702>"
)

target_include_directories(
  ${PROJECT_NAME} INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
                            $<INSTALL_INTERFACE:include/${PROJECT_NAME}-${PROJECT_VERSION}>
)

# ---- Create an installable target ----
# this allows users to install and find the library via `find_package()`.

# the location where the project's version header will be placed should match the project's regular
# header paths
string(TOLOWER ${PROJECT_NAME}/version.h VERSION_HEADER_LOCATION)

packageProject(
  NAME ${PROJECT_NAME}
  VERSION ${PROJECT_VERSION}
  NAMESPACE ${PROJECT_NAME}
  BINARY_DIR ${PROJECT_BINARY_DIR}
  INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include
  INCLUDE_DESTINATION include/${PROJECT_NAME}-${PROJECT_VERSION}
  VERSION_HEADER "${VERSION_HEADER_LOCATION}"
  COMPATIBILITY SameMajorVersion
)

# ---------------------------------------------------------------------------
# Standalone executable
# ---------------------------------------------------------------------------

if(NETOPTIM_BUILD_STANDALONE)
  # cxxopts is a header-only CLI parser used by the standalone example. Prefer a consumer- or
  # system-provided cxxopts; only fall back to CPM when no cxxopts target is available.
  find_package(cxxopts CONFIG QUIET)
  if(NOT cxxopts_FOUND AND NOT TARGET cxxopts::cxxopts)
    CPMAddPackage(
      GITHUB_REPOSITORY jarro2783/cxxopts
      VERSION 3.2.1
      OPTIONS "CXXOPTS_BUILD_EXAMPLES NO" "CXXOPTS_BUILD_TESTS NO" "CXXOPTS_ENABLE_INSTALL NO"
    )
  endif()
  # keep third-party headers out of our -Werror scope
  if(TARGET cxxopts::cxxopts)
    get_target_property(CXXOPTS_INCLUDE_DIRS cxxopts::cxxopts INTERFACE_INCLUDE_DIRECTORIES)
  endif()

  file(GLOB NETOPTIM_STANDALONE_SOURCES CONFIGURE_DEPENDS "standalone/source/*.cpp")
  add_executable(NetOptimStandalone ${NETOPTIM_STANDALONE_SOURCES})
  set_target_properties(NetOptimStandalone PROPERTIES OUTPUT_NAME "NetOptim" CXX_STANDARD 20)
  target_include_directories(NetOptimStandalone SYSTEM PRIVATE ${CXXOPTS_INCLUDE_DIRS})
  target_link_libraries(NetOptimStandalone PRIVATE NetOptim::NetOptim cxxopts::cxxopts
                                                 ${SPECIFIC_LIBS}
  )
  target_enable_warnings(NetOptimStandalone)
  target_enable_abseil_suppressions(NetOptimStandalone)
endif()

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------

enable_testing()

if(NETOPTIM_BUILD_TESTS)
  # Try system-installed doctest first (Ubuntu: doctest-dev, macOS: brew install doctest, Termux:
  # doctest)
  find_package(doctest CONFIG QUIET)

  if(NOT doctest_FOUND)
    CPMAddPackage(
      NAME doctest
      GITHUB_REPOSITORY doctest/doctest
      VERSION 2.5.2
      OPTIONS "DOCTEST_NO_INSTALL ON" # keep the installed prefix minimal
    )
  endif()

  # Code formatting targets (format / fix-format) via Format.cmake
  CPMAddPackage("gh:TheLartians/Format.cmake@1.7.3")

  file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS "test/source/*.cpp")
  add_executable(NetOptimTests ${TEST_SOURCES})
  target_link_libraries(NetOptimTests PRIVATE doctest::doctest NetOptim::NetOptim ${SPECIFIC_LIBS}
                                              ${SPECIFIC_ABSEIL_LIBS}
  )
  set_target_properties(NetOptimTests PROPERTIES CXX_STANDARD 20)
  target_enable_warnings(NetOptimTests)
  target_enable_abseil_suppressions(NetOptimTests)
  add_test(NAME netoptimTests COMMAND NetOptimTests)
endif()

# ---------------------------------------------------------------------------
# Benchmarks
# ---------------------------------------------------------------------------

if(NETOPTIM_BUILD_BENCHMARKS)
  # nanobench is a header-only benchmarking library
  set(NANOBENCH_VERSION 4.3.11)
  set(NANOBENCH_DIR "${CMAKE_BINARY_DIR}/_deps/nanobench")
  set(NANOBENCH_HEADER "${NANOBENCH_DIR}/nanobench.h")

  if(NOT EXISTS "${NANOBENCH_HEADER}")
    file(MAKE_DIRECTORY "${NANOBENCH_DIR}")
    file(
      DOWNLOAD
      "https://raw.githubusercontent.com/martinus/nanobench/v${NANOBENCH_VERSION}/src/include/nanobench.h"
      "${NANOBENCH_HEADER}"
      STATUS download_status
    )
    list(GET download_status 0 status_code)
    if(NOT status_code EQUAL 0)
      message(FATAL_ERROR "Failed to download nanobench (status ${status_code})")
    endif()
  endif()

  add_library(nanobench INTERFACE)
  target_include_directories(nanobench SYSTEM INTERFACE "${NANOBENCH_DIR}")

  file(GLOB_RECURSE ALL_BENCH_CPP CONFIGURE_DEPENDS "benchmark/source/*.cpp")
  foreach(ONE_BENCH_CPP ${ALL_BENCH_CPP})
    get_filename_component(ONE_BENCH_EXEC ${ONE_BENCH_CPP} NAME_WE)
    set(TARGET_NAME Bench_${ONE_BENCH_EXEC})

    add_executable(${TARGET_NAME} ${ONE_BENCH_CPP})
    set_target_properties(${TARGET_NAME} PROPERTIES OUTPUT_NAME ${ONE_BENCH_EXEC} CXX_STANDARD 20)
    target_link_libraries(${TARGET_NAME} PRIVATE nanobench NetOptim::NetOptim ${SPECIFIC_LIBS}
                                                ${SPECIFIC_ABSEIL_LIBS}
    )
    target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_enable_warnings(${TARGET_NAME})
    target_enable_abseil_suppressions(${TARGET_NAME})
    add_test(NAME ${ONE_BENCH_EXEC} COMMAND ${TARGET_NAME})
  endforeach()
endif()

# ---------------------------------------------------------------------------
# clang-tidy
# ---------------------------------------------------------------------------

if(NETOPTIM_ENABLE_CLANG_TIDY)
  find_program(CLANG_TIDY clang-tidy)
  if(CLANG_TIDY)
    add_custom_target(
      clang-tidy
      COMMAND ${CLANG_TIDY} --extra-arg=-std=c++20
              --extra-arg=-I${CMAKE_CURRENT_SOURCE_DIR}/include
              ${CMAKE_CURRENT_SOURCE_DIR}/include/netoptim/*.hpp
      COMMENT "Running clang-tidy static analysis"
    )
  else()
    message(WARNING "clang-tidy not found - analysis target unavailable")
  endif()
endif()

# ---------------------------------------------------------------------------
# Doxygen documentation
# ---------------------------------------------------------------------------

if(NETOPTIM_BUILD_DOCS)
  find_package(Doxygen QUIET)
  if(DOXYGEN_FOUND)
    set(DOXYGEN_PROJECT_NAME ${PROJECT_NAME})
    set(DOXYGEN_PROJECT_VERSION ${PROJECT_VERSION})
    set(DOXYGEN_PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
    set(DOXYGEN_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/doxygen")

    configure_file(documentation/Doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    add_custom_target(
      GenerateDocs
      COMMAND ${DOXYGEN_EXECUTABLE} "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
      COMMAND echo "Docs written to: ${DOXYGEN_OUTPUT_DIRECTORY}/html"
      COMMENT "Generating Doxygen HTML documentation in ${DOXYGEN_OUTPUT_DIRECTORY}/html"
      WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    )
  else()
    add_custom_target(
      GenerateDocs COMMAND ${CMAKE_COMMAND} -E echo
                           "Doxygen not found - skipping documentation generation"
    )
  endif()
endif()

# ---------------------------------------------------------------------------
# Coverage report target (GCC/Clang only - requires gcovr)
# ---------------------------------------------------------------------------

if(NETOPTIM_ENABLE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
  # NetOptim is a header-only INTERFACE library - coverage flags go on the executables that
  # instantiate the templates.
  foreach(COVERAGE_TARGET NetOptimTests NetOptimStandalone)
    if(TARGET ${COVERAGE_TARGET})
      target_compile_options(${COVERAGE_TARGET} PRIVATE -O0 -g -fprofile-arcs -ftest-coverage)
    endif()
  endforeach()
  foreach(COVERAGE_EXEC NetOptimTests NetOptimStandalone)
    if(TARGET ${COVERAGE_EXEC})
      target_link_options(${COVERAGE_EXEC} PRIVATE -fprofile-arcs -ftest-coverage)
    endif()
  endforeach()

  find_program(GCOVR_PATH gcovr)
  if(GCOVR_PATH)
    set(extra_gcovr_args "")
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang")
      find_program(LLVM_COV_PATH llvm-cov)
      if(LLVM_COV_PATH)
        list(APPEND extra_gcovr_args --llvm-cov-binary ${LLVM_COV_PATH})
      endif()
    endif()
    add_custom_target(
      coverage
      COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
      COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/coverage
      COMMAND ${GCOVR_PATH} --root ${PROJECT_SOURCE_DIR} --html-details
              ${CMAKE_BINARY_DIR}/coverage/index.html --exclude test/ --exclude build/
              ${extra_gcovr_args}
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
      COMMENT "Generating coverage report in ${CMAKE_BINARY_DIR}/coverage/index.html"
    )
  else()
    message(WARNING "gcovr not found - coverage target unavailable. Install with: pip install gcovr")
  endif()
endif()
