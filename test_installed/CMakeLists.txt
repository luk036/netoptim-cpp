cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# ---- Include guards ----

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(
    FATAL_ERROR
      "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
  )
endif()

project(NetOptim LANGUAGES CXX)

# --- Import tools ----

include(../cmake/tools.cmake)

# ---- Dependencies (all installed by the parent build) ----

include(../cmake/CPM.cmake)

# Find dependencies first — transitive dep configs reference fmt::fmt, spdlog::spdlog etc.
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)
find_package(Py2Cpp REQUIRED)
find_package(XNetwork REQUIRED)
find_package(MyWheel REQUIRED)
find_package(DiGraphX REQUIRED)
find_package(EllAlgo REQUIRED)
find_package(${PROJECT_NAME} REQUIRED)

set(SPECIFIC_LIBS EllAlgo::EllAlgo DiGraphX::DiGraphX XNetwork::XNetwork Py2Cpp::Py2Cpp
                  Threads::Threads fmt::fmt
)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../standalone ${CMAKE_BINARY_DIR}/test_installed)
