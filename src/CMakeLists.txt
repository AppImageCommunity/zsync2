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
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)
# since there's headers in the build directory now, include that as well
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(ZSYNC2_DEPS ${CPR_LIBRARIES} libzsync)
set(ZSYNC2_HDRS
    format_string.h
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
    ${PROJECT_SOURCE_DIR}/include/zsutil.h
    ${PROJECT_SOURCE_DIR}/include/zsclient.h
    ${PROJECT_SOURCE_DIR}/include/zsglobal.h
    ${PROJECT_SOURCE_DIR}/include/zsmake.h
)
set(ZSYNC2_SRCS zsclient.cpp legacy_http.c legacy_progress.c zsmake.cpp)

# core library
add_library(libzsync2 ${ZSYNC2_SRCS})
# since the target is called libsomething, one doesn't need CMake's additional lib prefix
set_target_properties(libzsync2 PROPERTIES PREFIX "")
target_link_libraries(libzsync2 PRIVATE ${ZSYNC2_DEPS})
target_include_directories(libzsync2 PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)

# core executable
add_executable(zsync2 ${ZSYNC2_SRCS} ${ZSYNC2_HDRS} main.cpp)
target_include_directories(zsync2 PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(zsync2 PRIVATE ${ZSYNC2_DEPS} args)
target_compile_definitions(zsync2 PRIVATE -DZSYNC_STANDALONE)

## zsyncmake2 executable
add_executable(zsyncmake2 ${ZSYNC2_SRCS} ${ZSYNC2_HDRS} make_main.cpp)
target_link_libraries(zsyncmake2 ${ZSYNC2_DEPS} args)

# install targets
install(
    TARGETS zsync2 libzsync2 zsyncmake2
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib/static
    INCLUDES DESTINATION include
)
