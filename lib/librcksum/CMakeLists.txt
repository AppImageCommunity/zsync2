cmake_minimum_required(VERSION 3.2)

project(librcksum)

# there are tests, so enable testing
enable_testing()

# add actual library
add_library(librcksum STATIC rsum.c hash.c state.c range.c md4.c internal.h rcksum.h md4.h)
# since the target is called libsomething, one doesn't need CMake's additional lib prefix
set_target_properties(librcksum PROPERTIES PREFIX "")
# set includes
target_include_directories(librcksum INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")

# add tests
add_executable(md4test md4test.c md4.c)
add_test(md4test md4test)
