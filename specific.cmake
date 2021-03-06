set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

cpmaddpackage(
  NAME
  fmt
  GIT_TAG
  7.1.3
  GITHUB_REPOSITORY
  fmtlib/fmt
  OPTIONS
  "FMT_INSTALL YES" # create an installable target
)

find_package(Boost REQUIRED)
if(Boost_FOUND)
  message(STATUS "Found boost: ${Boost_INCLUDE_DIRS}")
  # add_library(Boost::boost INTERFACE IMPORTED GLOBAL)
  target_include_directories(Boost::boost SYSTEM
                             INTERFACE ${Boost_INCLUDE_DIRS})
  # Disable autolink
  target_compile_definitions(Boost::boost INTERFACE BOOST_ALL_NO_LIB=1)
endif()

cpmaddpackage(NAME cppcoro GIT_TAG 1.0 GITHUB_REPOSITORY luk036/cppcoro)
# print_target_properties(cppcoro)
if(cppcoro_ADDED)
  message(STATUS "Found cppcoro: ${cppcoro_SOURCE_DIR}")
  add_library(cppcoro::cppcoro INTERFACE IMPORTED GLOBAL)
  target_include_directories(cppcoro::cppcoro SYSTEM
                             INTERFACE ${cppcoro_SOURCE_DIR}/include)
endif(cppcoro_ADDED)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # using GCC
  add_compile_options(-fcoroutines)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  # using clang
  add_compile_options(-fcoroutines-ts -stdlib=libc++)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  # using Visual Studio C++
  add_compile_options(/std:c++latest /await)
endif()

cpmaddpackage(
  NAME
  Py2Cpp
  GIT_TAG
  1.2
  GITHUB_REPOSITORY
  luk036/py2cpp
  OPTIONS
  "INSTALL_ONLY ON" # create an installable target
)

cpmaddpackage(
  NAME
  XNetwork
  GIT_TAG
  1.5
  GITHUB_REPOSITORY
  luk036/xnetwork-cpp
  OPTIONS
  "INSTALL_ONLY ON" # create an installable target
)

cpmaddpackage("gh:xtensor-stack/xtl#0.6.23")
if(xtl_ADDED)
  message(STATUS "Found xtl: ${xtl_SOURCE_DIR}")
  include_directories(${xtl_SOURCE_DIR}/include)
endif(xtl_ADDED)

cpmaddpackage("gh:xtensor-stack/xtensor#0.22.0")
if(xtensor_ADDED)
  message(STATUS "Found xtensor: ${xtensor_SOURCE_DIR}")
  include_directories(${xtensor_SOURCE_DIR}/include)
endif(xtensor_ADDED)

cpmaddpackage(
  NAME
  EllAlgo
  GIT_TAG
  1.1
  GITHUB_REPOSITORY
  luk036/ellalgo-cpp
  OPTIONS
  "INSTALL_ONLY YES" # create an installable target
)

set(SPECIFIC_LIBS
    EllAlgo::EllAlgo
    XNetwork::XNetwork
    Py2Cpp::Py2Cpp
    Boost::boost
    cppcoro::cppcoro
    Threads::Threads
    fmt::fmt)
