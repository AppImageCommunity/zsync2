cmake_minimum_required(VERSION 3.11)

project(libzsync)

set(CMAKE_POSITION_INDEPENDENT_CODE true)

# there are tests, so enable testing
enable_testing()

# add actual library
add_library(libzsync OBJECT zsync.c zmap.c sha1.c)

# since the target is called libsomething, one doesn't need CMake's additional lib prefix
set_target_properties(libzsync PROPERTIES PREFIX "")

# link relevant libraries
target_link_libraries(libzsync PRIVATE zsync2_libz PUBLIC librcksum)

# declare includes
target_include_directories(libzsync PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

# needed for strptime to work
target_compile_definitions(libzsync PRIVATE _GNU_SOURCE _XOPEN_SOURCE)

# add tests
add_executable(sha1test sha1.c sha1test.c)
add_test(sha1test sha1test)
