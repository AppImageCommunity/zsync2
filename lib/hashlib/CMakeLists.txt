# minimal zlib-licensed hash library
# courtesy of Stephan Brumme

cmake_minimum_required(VERSION 3.2)

add_library(hashlib STATIC
    include/hashlib/md5.h
    src/md5.cpp
    include/hashlib/sha1.h
    src/sha1.cpp
    include/hashlib/sha256.h
    src/sha256.cpp
)
target_include_directories(hashlib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
set_target_properties(hashlib PROPERTIES PREFIX "")

add_executable(hashlib_tests tests/tests.cpp)
