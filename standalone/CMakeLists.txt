cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(NetOptimCppStandalone LANGUAGES CXX)

# --- Import tools ----

include(../cmake/tools.cmake)

# ---- Dependencies ----

include(../cmake/CPM.cmake)

CPMAddPackage(
  GITHUB_REPOSITORY jarro2783/cxxopts
  VERSION 2.2.1
  OPTIONS "CXXOPTS_BUILD_EXAMPLES NO" "CXXOPTS_BUILD_TESTS NO" "CXXOPTS_ENABLE_INSTALL YES"
)

# CPMAddPackage("gh:luk036/xn-cpp@1.0.1")

find_package(XNCpp REQUIRED)
# if (XNCpp_FOUND)
#   message(STATUS "Found XNCpp: ${XNCpp_INCLUDE_DIRS}")
# else()
#   CPMAddPackage("gh:luk036/xn-cpp@1.0.1")
# endif (XNCpp_FOUND)

find_package (Boost REQUIRED COMPONENTS coroutine context system)
if (Boost_FOUND)
    message(STATUS "Found boost: ${Boost_INCLUDE_DIRS}")
    include_directories(${Boost_INCLUDE_DIRS})
endif (Boost_FOUND)

CPMAddPackage(NAME NetOptimCpp SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/..)

# ---- Create standalone executable ----

file(GLOB sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)

add_executable(NetOptimCppStandalone ${sources})

set_target_properties(NetOptimCppStandalone PROPERTIES CXX_STANDARD 17 OUTPUT_NAME "NetOptimCpp")

target_link_libraries(NetOptimCppStandalone NetOptimCpp::NetOptimCpp cxxopts XNCpp::XNCpp ${Boost_LIBRARIES})
