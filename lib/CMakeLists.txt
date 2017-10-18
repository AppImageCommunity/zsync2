set(CMAKE_MINIMUM_REQUIRED_VERSION 3.2)

include_directories(.)

# include subdirectories in the right order
add_subdirectory(zlib)
add_subdirectory(librcksum)
add_subdirectory(libzsync)

# external dependencies used as submodules

# using system cURL as it has proper SSL support etc., and reduces the build time
set(USE_SYSTEM_CURL ON)
# disable CPR tests to save compile time and avoid clashes with own tests
set(BUILD_CPR_TESTS OFF)
add_subdirectory(cpr)

# it's quite simple dealing with the args library directly from CMake
add_library(args INTERFACE)
target_include_directories(args INTERFACE args)

# boost::boost_optional, imported from GitHub
# added a CMakeLists for convenience
add_subdirectory(optional)

# Google Test framework
add_subdirectory(gtest)
