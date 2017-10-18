cmake_minimum_required(VERSION 3.2)

project(librcksum)

# there are tests, so enable testing
enable_testing()

# add actual library
add_library(librcksum STATIC rsum.c hash.c state.c range.c md4.c)
# since the target is called libsomething, one doesn't need CMake's additional lib prefix
set_target_properties(librcksum PROPERTIES PREFIX "")

# add tests
add_executable(md4test md4test.c md4.c)
add_test(md4test md4test)
