cmake_minimum_required(VERSION 3.2)

project(zsync2)

# going to use CTest within the project
include(CTest)

# will perform platform checks
include(CheckFunctionExists)
include(CheckIncludeFiles)

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
include_directories(include)

# config.h contains some global defines, our config.h.in is a modified version of the upstream config.h.in to make it
# integrate with CMake
set(VERSION "2.0.0-alpha-1")
configure_file(src/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)
# since there's headers in the build directory now, include that as well
include_directories(${CMAKE_CURRENT_BINARY_DIR})

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

# add libraries shipped along with the project
add_subdirectory(lib)

# add source code
add_subdirectory(src)

# add unit tests
add_subdirectory(tests)