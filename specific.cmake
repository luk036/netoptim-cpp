set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Try system-installed fmt first (Ubuntu: libfmt-dev, macOS: brew install fmt, Termux: fmt)
find_package(fmt CONFIG QUIET)

if(fmt_FOUND)
  message(STATUS "Found system fmt: ${fmt_DIR}")
  # Tell CPM that fmt is already handled (CPM checks CPM_PACKAGES list). Write the CACHE
  # variable directly: list(APPEND ...) creates a normal-variable shadow that does not
  # propagate into FetchContent subdirectory scopes.
  if(NOT fmt IN_LIST CPM_PACKAGES)
    set(CPM_PACKAGES "${CPM_PACKAGES};fmt" CACHE INTERNAL "" FORCE)
  endif()
else()
  CPMAddPackage(
    NAME fmt
    GIT_TAG 12.1.0
    GITHUB_REPOSITORY fmtlib/fmt
    OPTIONS "FMT_INSTALL YES" # create an installable target
  )
endif()

# find_package(Boost REQUIRED) if(Boost_FOUND) message(STATUS "Found boost: ${Boost_INCLUDE_DIRS}")
# # add_library(Boost::boost INTERFACE IMPORTED GLOBAL) target_include_directories(Boost::boost
# SYSTEM INTERFACE ${Boost_INCLUDE_DIRS}) # Disable autolink target_compile_definitions(Boost::boost
# INTERFACE BOOST_ALL_NO_LIB=1) endif()

# CPMAddPackage( NAME cppcoro GIT_TAG 2.0 GITHUB_REPOSITORY luk036/cppcoro ) #
# print_target_properties(cppcoro) if(cppcoro_ADDED) message(STATUS "Found cppcoro:
# ${cppcoro_SOURCE_DIR}") add_library(cppcoro::cppcoro INTERFACE IMPORTED GLOBAL)
# target_include_directories(cppcoro::cppcoro SYSTEM INTERFACE ${cppcoro_SOURCE_DIR}/include)
# endif(cppcoro_ADDED)
#
# if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU") # using GCC add_compile_options(-fcoroutines)
# elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang") # using clang add_compile_options(-fcoroutines-ts
# -stdlib=libc++) elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC") # using Visual Studio C++
# add_compile_options(/std:c++latest /await) endif()

CPMAddPackage(
  NAME Py2Cpp
  GIT_TAG v1.6.3
  GITHUB_REPOSITORY luk036/py2cpp
  OPTIONS "INSTALL_ONLY ON" # create an installable target
)

# Suppress MSVC STL1011 error on <experimental/coroutine> used by xnetwork via /await
if(MSVC)
  add_compile_definitions(_SILENCE_EXPERIMENTAL_COROUTINE_DEPRECATION_WARNINGS)
endif()

CPMAddPackage(
  NAME XNetwork
  GIT_TAG v1.7.6
  GITHUB_REPOSITORY luk036/xnetwork-cpp
  OPTIONS "INSTALL_ONLY ON" # create an installable target
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

# spdlog must be added before DiGraphX (which also adds spdlog) to ensure SPDLOG_FMT_EXTERNAL is
# set, avoiding linker conflicts with fmt when local fmt package is a shared library
# Try system-installed spdlog first (Ubuntu: libspdlog-dev, macOS: brew install spdlog, Termux: spdlog)
find_package(spdlog CONFIG QUIET)

if(spdlog_FOUND)
  message(STATUS "Found system spdlog: ${spdlog_DIR}")
  # Tell CPM that spdlog is already handled (write CACHE directly, see fmt above)
  if(NOT spdlog IN_LIST CPM_PACKAGES)
    set(CPM_PACKAGES "${CPM_PACKAGES};spdlog" CACHE INTERNAL "" FORCE)
  endif()
else()
  CPMAddPackage(
    NAME spdlog
    GIT_TAG v1.17.0
    GITHUB_REPOSITORY gabime/spdlog
    OPTIONS "SPDLOG_INSTALL YES" "SPDLOG_FMT_EXTERNAL YES"
  )
endif()

CPMAddPackage(
  NAME DiGraphX
  GIT_TAG v1.1.6
  GITHUB_REPOSITORY luk036/digraphx-cpp
  OPTIONS "INSTALL_ONLY ON" # create an installable target
)

CPMAddPackage(
  NAME EllAlgo
  GIT_TAG v1.6.8
  GITHUB_REPOSITORY luk036/ellalgo-cpp
  OPTIONS "INSTALL_ONLY YES" # create an installable target
)

# Set C++ standard at project level (abseil requires this at configure time)
set(CMAKE_CXX_STANDARD 20)

# DiGraphX (fetched above) already adds abseil via CPM — use its REAL targets if present
# instead of mixing with find_package IMPORTED targets (IMPORTED vs ALIAS conflict).
if(TARGET absl::flat_hash_map)
  message(STATUS "Using CPM-provided abseil (from DiGraphX)")
  set(SPECIFIC_ABSEIL_LIBS absl::flat_hash_map)
else()
  # Try system-installed abseil first (Ubuntu: libabsl-dev, macOS: brew install abseil,
  # Termux: pkg install abseil-cpp). Falls back to CPM build on Windows or when no system
  # package is available.
  find_package(absl CONFIG QUIET)

  if(absl_FOUND)
    message(STATUS "Found system abseil: ${absl_DIR}")
    set(SPECIFIC_ABSEIL_LIBS absl::flat_hash_map)
  else()
    # Add abseil for flat_hash_map (used in test/benchmark files)
    CPMAddPackage(
      NAME abseil-cpp
      GIT_TAG 20260107.1
      GITHUB_REPOSITORY abseil/abseil-cpp
      OPTIONS "ABSL_PROPAGATE_CXX_STD ON"
    )
    # Abseil targets to link to executables (not the library, to avoid export set conflicts)
    set(SPECIFIC_ABSEIL_LIBS absl::flat_hash_map)
  endif()
endif()

set(SPECIFIC_LIBS
    EllAlgo::EllAlgo DiGraphX::DiGraphX XNetwork::XNetwork Py2Cpp::Py2Cpp
    # cppcoro::cppcoro
    Threads::Threads fmt::fmt
)
