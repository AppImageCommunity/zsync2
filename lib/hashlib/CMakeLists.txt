# minimal zlib-licensed hash library
# courtesy of Stephan Brumme

cmake_minimum_required(VERSION 3.2)

set(HASHLIB_SRCS
    src/crc32.cpp
    src/digest.cpp
    src/keccak.cpp
    src/md5.cpp
    src/sha1.cpp
    src/sha256.cpp
    src/sha3.cpp
)
set(HASHLIB_HDRS
    include/hashlib/crc32.h
    include/hashlib/hash.h
    include/hashlib/hmac.h
    include/hashlib/keccak.h
    include/hashlib/md5
    include/hashlib/sha1.h
    include/hashlib/sha256.h
    include/hashlib/sha3.h
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
