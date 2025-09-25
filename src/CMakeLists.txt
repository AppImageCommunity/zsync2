set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE true)

set(zsync2_public_headers
    ${PROJECT_SOURCE_DIR}/include/zsutil.h
    ${PROJECT_SOURCE_DIR}/include/zsclient.h
    ${PROJECT_SOURCE_DIR}/include/zsglobal.h
    ${PROJECT_SOURCE_DIR}/include/zsmake.h
    ${PROJECT_SOURCE_DIR}/include/zshash.h
)

# at the moment, we need to build libzsync2 twice because of the ZSYNC_STANDALONE macro which controls logging
# one is the public use library with the alternative status message collection system, the other one is just used in
# the zsync2 target
function(add_libzsync2 NAME BUILD_TYPE)
    add_library("${NAME}" "${BUILD_TYPE}"
        zsclient.cpp
        legacy_http.c
        legacy_progress.c
        zsmake.cpp
        zsutil.cpp
        format_string.h
        ${PROJECT_BINARY_DIR}/config.h
        ${zsync2_public_headers}
    )
    set_target_properties("${NAME}" PROPERTIES
        PREFIX ""
        PUBLIC_HEADER "${zsync2_public_headers}"
    )
    target_link_libraries("${NAME}"
        PRIVATE cpr libzsync
        # needed for public header-only lib zshash.h
        PUBLIC PkgConfig::libgcrypt
    )
    target_include_directories("${NAME}"
        PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
        INTERFACE "$<INSTALL_INTERFACE:include>"
    )
    set_target_properties("${NAME}" PROPERTIES INSTALL_RPATH "\$ORIGIN")

    if(LIBCURL_FOUND)
        target_compile_definitions("${NAME}" PRIVATE HAVE_LIBCURL=1)
    endif()
    if(LIBSOCKET_FOUND)
        target_compile_definitions("${NAME}" PRIVATE HAVE_LIBSOCKET=1)
    endif()

    # largefile support à la gpgme
    # libzsync uses off_t everywhere, this allows us to support files larger than 2GiB
    # must(!) be public definitions, otherwise integrating libzsync2 elsewhere (e.g., AppImageUpdate) may lead to
    # compiler errors because the off_t definitions differ
    target_compile_definitions("${NAME}"
        PUBLIC -D_FILE_OFFSET_BITS=64
        PUBLIC -DLARGEFILE_SOURCE
    )
endfunction()

add_libzsync2(libzsync2 SHARED)

# for use as a submodule
add_libzsync2(libzsync2_static STATIC)
set_property(TARGET libzsync2_static PROPERTY OUTPUT_NAME libzsync2)

add_libzsync2(libzsync2_standalone STATIC)
# this will propagate to target zsync, too
target_compile_definitions(libzsync2_standalone PUBLIC ZSYNC_STANDALONE)

# core executable
add_executable(zsync2 main.cpp)
target_link_libraries(zsync2 PRIVATE libzsync2_standalone args)
# TODO: get rid of this
target_link_libraries(zsync2 PRIVATE cpr)
set_target_properties(zsync2 PROPERTIES INSTALL_RPATH "\$ORIGIN/../${CMAKE_INSTALL_LIBDIR}")

## zsyncmake2 executable
add_executable(zsyncmake2 make_main.cpp)
target_link_libraries(zsyncmake2 PRIVATE libzsync2 args)
# TODO: get rid of this
target_link_libraries(zsyncmake2 PRIVATE cpr)
set_target_properties(zsyncmake2 PROPERTIES INSTALL_RPATH "\$ORIGIN/../${CMAKE_INSTALL_LIBDIR}")

# install targets
install(
    TARGETS libzsync2
    EXPORT zsync2Targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT libzsync2
)

install(
    TARGETS zsync2 zsyncmake2
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT zsync2
)
