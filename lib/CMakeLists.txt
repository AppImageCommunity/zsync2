# for pkg-config IMPORTED_TARGET
cmake_minimum_required(VERSION 3.6)

include_directories(.)

# include subdirectories in the right order
add_subdirectory(zlib)
add_subdirectory(librcksum)
add_subdirectory(libzsync)

# note: we use functions to import the dependencies to have separate scopes for the variables we apply on them
# https://alexreinking.com/blog/building-a-dual-shared-and-static-library-with-cmake.html
if(NOT USE_SYSTEM_CPR)
    function(import_cpr)
        # for some reason we don't know, cpr wants to build shared libs by default
        # this is not the default behavior of CMake, so we force this option off again
        # unfortunately, we have to do this globally...
        # long term goal is to replace the submodule in some way, e.g., by using prebuilt cpr binaries
        set(BUILD_SHARED_LIBS OFF)

        # make sure that static libraries (especially built cURL libs) can be linked properly
        set(CMAKE_POSITION_INDEPENDENT_CODE ON)

        # there is no need to build our own libcurl
        # in fact, building it as a static lib creates a whole circus of problems, and also takes time

        # if a system provided installed libcurl is too "new", we want to have CPR build its own
        # otherwise, we want to use the system provided library
        # this way, we can build against a system-wide libcurl on older systems (e.g., Ubuntu focal) while
        # allowing development on newer systems
        # see https://github.com/libcpr/cpr/issues/804
        # note: don't use find_package(CURL), for some reason it'd cause zsync2 to link to the system libcurl in
        # addition to the statically linked CPR; we just need the version
        find_package(PkgConfig REQUIRED)
        pkg_search_module(PKGCONFIG_CURL libcurl)
        set(PKGCONFIG_CURL_FOUND ${PKGCONFIG_CURL_FOUND} PARENT_SCOPE)
        set(PKGCONFIG_CURL_VERSION ${PKGCONFIG_CURL_VERSION} PARENT_SCOPE)

        if(PKGCONFIG_CURL_FOUND)
            message(STATUS "Found libcurl ${PKGCONFIG_CURL_VERSION}")

            if(NOT DEFINED CPR_FORCE_USE_SYSTEM_CURL)
                if (PKGCONFIG_CURL_VERSION VERSION_LESS_EQUAL 7.80.0)
                    message(STATUS "libcurl is old enough to work with CPR, so we use the system one (use -DCPR_FORCE_USE_SYSTEM_CURL=OFF to change this behavior)")
                    set(CPR_FORCE_USE_SYSTEM_CURL ON)
                else()
                    message(WARNING "libcurl is too new to work with CPR, so we let CPR build its own (use -DCPR_FORCE_USE_SYSTEM_CURL=ON to change this behavior)")
                    set(CPR_FORCE_USE_SYSTEM_CURL OFF)
                endif()
            else()
                message(WARNING "CPR_FORCE_USE_SYSTEM_CURL is set to ${CPR_FORCE_USE_SYSTEM_CURL} by the user")
            endif()
        else()
            message(WARNING "Could not find libcurl with pkg-config, relying on CPR auto-configuration")
        endif()

        include(FetchContent)
        FetchContent_Declare(cpr
            GIT_REPOSITORY https://github.com/libcpr/cpr.git
            # note: newer versions don't necessarily work with Ubuntu bionic's libcurl packages!
            GIT_TAG 1.8.3
        )
        FetchContent_MakeAvailable(cpr)

        # since we want to have static builds of cpr, we have to enable -fPIC manually
    #    set_property(TARGET cpr::cpr PROPERTY POSITION_INDEPENDENT_CODE ON)
    endfunction()

    import_cpr()
endif()

if(NOT USE_SYSTEM_ARGS)
    function(import_args)
        set(ARGS_MAIN_PROJECT OFF)
        set(ARGS_BUILD_EXAMPLE OFF)
        set(ARGS_BUILD_UNITTESTS OFF)

        include(FetchContent)
        FetchContent_Declare(args
            GIT_REPOSITORY https://github.com/Taywee/args
            GIT_TAG 6.4.6
        )
        FetchContent_MakeAvailable(args)
    endfunction()

    import_args()
endif()
