# minimal zlib-licensed hash library
# courtesy of Stephan Brumme

cmake_minimum_required(VERSION 3.2)

project(hashlib)

set(CMAKE_POSITION_INDEPENDENT_CODE true)

include_directories(${PROJECT_SOURCE_DIR}/include)

set(HASHLIB_SRCS
    ${PROJECT_SOURCE_DIR}/src/crc32.cpp
    ${PROJECT_SOURCE_DIR}/src/digest.cpp
    ${PROJECT_SOURCE_DIR}/src/keccak.cpp
    ${PROJECT_SOURCE_DIR}/src/md5.cpp
    ${PROJECT_SOURCE_DIR}/src/sha1.cpp
    ${PROJECT_SOURCE_DIR}/src/sha256.cpp
    ${PROJECT_SOURCE_DIR}/src/sha3.cpp
)
set(HASHLIB_HDRS
    ${PROJECT_SOURCE_DIR}/include/hashlib/crc32.h
    ${PROJECT_SOURCE_DIR}/include/hashlib/hash.h
    ${PROJECT_SOURCE_DIR}/include/hashlib/hmac.h
    ${PROJECT_SOURCE_DIR}/include/hashlib/keccak.h
    ${PROJECT_SOURCE_DIR}/include/hashlib/md5
    ${PROJECT_SOURCE_DIR}/include/hashlib/sha1.h
    ${PROJECT_SOURCE_DIR}/include/hashlib/sha256.h
    ${PROJECT_SOURCE_DIR}/include/hashlib/sha3.h
)

add_library(hashlib STATIC
    ${HASHLIB_SRCS}
    ${HASHLIB_HEADERS}
)
target_include_directories(hashlib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
set_target_properties(hashlib PROPERTIES PREFIX "")

set(BUILD_HASHLIB_TESTS ON CACHE BOOL "")

if(BUILD_HASHLIB_TESTS)
    enable_testing()

    message(STATUS "Building hashlib tests")

    add_executable(hashlib_tests tests/tests.cpp)
    #target_include_directories(hashlib_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_link_libraries(hashlib_tests PRIVATE hashlib)

    add_test(hashlib_tests hashlib_tests)
endif()
