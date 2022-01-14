cmake_minimum_required(VERSION 3.2)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE true)

# fix CPR target
target_include_directories(cpr PUBLIC ${CPR_INCLUDE_DIRS})

# core library
add_library(libzsync2 SHARED
    zsclient.cpp
    legacy_http.c
    legacy_progress.c
    zsmake.cpp
    ${ZSYNC2_SRCS} ${ZSYNC2_HDRS}
    format_string.h
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
    ${PROJECT_SOURCE_DIR}/include/zsutil.h
    ${PROJECT_SOURCE_DIR}/include/zsclient.h
    ${PROJECT_SOURCE_DIR}/include/zsglobal.h
    ${PROJECT_SOURCE_DIR}/include/zsmake.h
)
# since the target is called libsomething, one doesn't need CMake's additional lib prefix
set_target_properties(libzsync2 PROPERTIES PREFIX "")
target_link_libraries(libzsync2 PRIVATE cpr libzsync hashlib)
target_include_directories(libzsync2
    PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    INTERFACE $<INSTALL_INTERFACE:include>
)

if(LIBCURL_FOUND)
    target_compile_definitions(libzsync2 PRIVATE HAVE_LIBCURL=1)
endif()
if(LIBSOCKET_FOUND)
    target_compile_definitions(libzsync2 PRIVATE HAVE_LIBSOCKET=1)
endif()

# core executable
add_executable(zsync2 main.cpp)
target_link_libraries(zsync2 PRIVATE libzsync2 args)
target_compile_definitions(zsync2 PRIVATE -DZSYNC_STANDALONE)
# TODO: get rid of this
target_link_libraries(zsync2 PRIVATE cpr)

## zsyncmake2 executable
add_executable(zsyncmake2 make_main.cpp)
target_link_libraries(zsyncmake2 PRIVATE libzsync2 args)
# TODO: get rid of this
target_link_libraries(zsyncmake2 PRIVATE cpr)

# install targets
install(
    TARGETS zsync2 libzsync2 zsyncmake2
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
