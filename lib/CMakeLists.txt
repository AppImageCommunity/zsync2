# for pkg-config IMPORTED_TARGET
cmake_minimum_required(VERSION 3.6)

include_directories(.)

# include subdirectories in the right order
add_subdirectory(zlib)
add_subdirectory(librcksum)
add_subdirectory(libzsync)

# external dependencies used as submodules

if(NOT USE_SYSTEM_CPR)
    # for some reason we don't know, cpr wants to build shared libs by default
    # this is not the default behavior of CMake, so we force this option off again
    # unfortunately, we have to do this globally...
    # long term goal is to replace the submodule in some way, e.g., by using prebuilt cpr binaries
    set(BUILD_SHARED_LIBS OFF)

    # make sure that static libraries (especially built cURL libs) can be linked properly
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)

    # cannot use system cURL on bionic at the moment, PR with fix sent: https://github.com/libcpr/cpr/pull/696
    ## there is no need to build our own libcurl
    ## in fact, building it as a static lib creates a whole circus of problems, and also takes time
    #set(CPR_FORCE_USE_SYSTEM_CURL ON)

    add_subdirectory(cpr)

    # since we want to have static builds of cpr, we have to enable -fPIC manually
    set_property(TARGET cpr PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()

if(NOT USE_SYSTEM_ARGS)
    # it's quite simple dealing with the args library directly from CMake
    add_library(args INTERFACE)
    target_sources(args INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/args/args.hxx")
    target_include_directories(args INTERFACE args)
endif()

# Google Test framework
if(BUILD_TESTING AND NOT TARGET gtest)
    add_subdirectory(gtest EXCLUDE_FROM_ALL)
endif()
