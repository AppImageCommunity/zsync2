set(CMAKE_MINIMUM_REQUIRED_VERSION 3.2)

set(CMAKE_CXX_STANDARD 11)

if(LIBCURL_FOUND)
    add_definitions(-DHAVE_LIBCURL=1)
endif()
if(LIBSOCKET_FOUND)
    add_definitions(-DHAVE_LIBSOCKET=1)
endif()

# include cpr
include_directories(${CPR_INCLUDE_DIRS})

# config.h contains some global defines, our config.h.in is a modified version of the upstream config.h.in to make it
# integrate with CMake
set(VERSION "0.1.1")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)
# since there's headers in the build directory now, include that as well
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(ZSYNC2_DEPS ${CPR_LIBRARIES} libzsync)
set(ZSYNC2_SRCS zsclient.cpp legacy_http.c legacy_url.c legacy_progress.c)

# core library
add_library(libzsync2 ${ZSYNC2_SRCS})
# since the target is called libsomething, one doesn't need CMake's additional lib prefix
set_target_properties(libzsync2 PROPERTIES PREFIX "")
target_link_libraries(libzsync2 ${ZSYNC2_DEPS})
target_include_directories(libzsync2 PUBLIC ../include)

# core executable
add_executable(zsync2 main.cpp ${ZSYNC2_SRCS})
target_link_libraries(zsync2 ${ZSYNC2_DEPS} args)
target_compile_definitions(zsync2 PRIVATE -DZSYNC_STANDALONE)

# install targets
install(
    TARGETS zsync2 libzsync2
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib/static
)

## zsyncmake2 executable
#add_executable(zsyncmake2 make.cpp)
#target_link_libraries(zsyncmake2 libzsync2 args optional)
