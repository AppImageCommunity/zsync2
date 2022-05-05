cmake_minimum_required(VERSION 3.2)

project(zsync2)

# going to use CTest within the project
include(CTest)

# will perform platform checks
include(CheckFunctionExists)
include(CheckIncludeFiles)

# install into proper lib dirs on Linux
include(GNUInstallDirs)

# platform checks
foreach(header inttypes memory stdint stdlib strings sys/stat sys/types unistd time)
    string(REPLACE "/" "_" underscore_header "${header}")
    string(TOUPPER ${underscore_header} upper_header)
    check_include_files(${header}.h HAVE_${upper_header})
    if(HAVE_${upper_header})
        add_definitions(-DHAVE_${upper_header}_H=1)
    endif()
endforeach()

foreach(function fseeko getaddrinfo memcpy mkstemp pread pwrite)
    string(TOUPPER ${function} upper_function)
    check_function_exists(${function} HAVE_${upper_function})
    if(HAVE_${upper_function})
        add_definitions(-DHAVE_${upper_function}=1)
    endif()
endforeach()

# need to add include directory globally because of zsglobal.h
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# versioning data
set(VERSION "2.0.0-alpha-1")

if("$ENV{GITHUB_RUN_NUMBER}" STREQUAL "")
    set(BUILD_NUMBER "<local dev build>")
else()
    set(BUILD_NUMBER "$ENV{GITHUB_RUN_NUMBER}")
endif()

# read Git revision ID
# WARNING: this value will be stored in the CMake cache
# to update it, you will have to reset the CMake cache
# (doesn't matter for CI builds like Travis for instance, where there's no permanent CMake cache)
execute_process(
    COMMAND git rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# get current date
execute_process(
    COMMAND env LC_ALL=C date -u "+%Y-%m-%d %H:%M:%S %Z"
    OUTPUT_VARIABLE BUILD_DATE
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# config.h contains some global defines, our config.h.in is a modified version of the upstream config.h.in to make it
# integrate with CMake
configure_file(src/config.h.in ${PROJECT_BINARY_DIR}/config.h)
# since there's headers in the build directory now, include that as well
include_directories(${PROJECT_BINARY_DIR})

# find out which platform we're building for
if(UNIX)
    string(TOLOWER "${CMAKE_SYSTEM_NAME}" LOWER_SYSTEM)
    add_definitions("-DAPPIMAGEUPDATE_UNIX")
    if(APPLE)
        add_definitions("-DAPPIMAGEUPDATE_MACOS")
    elseif("${LOWER_SYSTEM}" MATCHES "bsd")
        add_definitions("-DAPPIMAGEUPDATE_BSD")
    elseif("${LOWER_SYSTEM}" MATCHES "linux")
        add_definitions("-DAPPIMAGEUPDATE_LINUX")
    else()
        message(FATAL_ERROR "Unsupported UNIX platform: ${CMAKE_SYSTEM_NAME}")
    endif()
else()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()

# doing this on project level allows all subdirs to use the imported targets
find_package(PkgConfig)

# we used to ship a minimal, zlib-licensed hash lib as part of zsync2
# now, we use a system provided crypto library for this purpose
pkg_check_modules(libgcrypt REQUIRED IMPORTED_TARGET libgcrypt)

option(USE_SYSTEM_CPR OFF "Use system-wide installed CPR")
option(USE_SYSTEM_ARGS OFF "Use system-wide installed args")

# makes linking to static libraries easier
option(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(USE_SYSTEM_CPR)
    find_package(cpr REQUIRED)
    add_library(cpr ALIAS cpr::cpr)
endif()

if(USE_SYSTEM_ARGS)
    find_package(args REQUIRED)

    # need to create an alias target to ease linking
    add_library(args ALIAS taywee::args)
endif()

# add libraries shipped along with the project
add_subdirectory(lib)

# add source code
add_subdirectory(src)

# add unit tests
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

# finally, generate export configs
include(cmake/export.cmake)
