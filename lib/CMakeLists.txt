# for pkg-config IMPORTED_TARGET
cmake_minimum_required(VERSION 3.6)

include_directories(.)

# include subdirectories in the right order
add_subdirectory(zlib)
add_subdirectory(librcksum)
add_subdirectory(libzsync)

# external dependencies used as submodules

if(NOT USE_SYSTEM_CPR)
    add_subdirectory(cpr)
    set_property(TARGET cpr PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()

# it's quite simple dealing with the args library directly from CMake
add_library(args INTERFACE)
target_sources(args INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/args/args.hxx")
target_include_directories(args INTERFACE args)

# Google Test framework
if(NOT TARGET gtest)
    add_subdirectory(gtest EXCLUDE_FROM_ALL)
endif()
