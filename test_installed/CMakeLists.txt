cmake_minimum_required(VERSION 3.14)
project(NetOptimInstalledTest LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find dependencies first - the installed configs reference Threads, fmt, spdlog, Py2Cpp, XNetwork,
# DiGraphX and EllAlgo. spdlog must be found before DiGraphX (its config references spdlog::spdlog).
# abseil is required by the headers.
find_package(Threads REQUIRED)
find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)
find_package(Py2Cpp REQUIRED)
find_package(XNetwork REQUIRED)
find_package(DiGraphX REQUIRED)
find_package(EllAlgo REQUIRED)
find_package(absl REQUIRED)
find_package(NetOptim REQUIRED)

add_executable(netoptim_installed_test main.cpp)
target_link_libraries(netoptim_installed_test PRIVATE NetOptim::NetOptim absl::flat_hash_map)

enable_testing()
add_test(NAME netoptim_installed_test COMMAND netoptim_installed_test)
