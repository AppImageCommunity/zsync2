# make sure CMake won't export this package by default (valid from 3.15 onwards)
cmake_minimum_required(VERSION 3.15)
set(CMP0090 NEW)

# set up installion of exported targets (consider libzsync2 a namespace for use in CMake)
install(
    TARGETS libzsync2
    DESTINATION lib
    EXPORT libzsync2
)

# allow import from current build tree
export(
    TARGETS libzsync2
    NAMESPACE zsync2::
    FILE libzsync2Targets.cmake
)

# allow import from install tree
# note that the targets must be install(... EXPORT zsync) in order for this to work (consider libzsync2 a namespace)
install(
    EXPORT libzsync2
    DESTINATION lib/cmake/zsync2
)

include(CMakePackageConfigHelpers)
# generate the config file that is includes the exports
configure_package_config_file(
    "${CMAKE_CURRENT_LIST_DIR}/zsync2Config.cmake.in"
    "${PROJECT_BINARY_DIR}/cmake/zsync2Config.cmake"
    INSTALL_DESTINATION "lib/cmake/zsync2"
    NO_SET_AND_CHECK_MACRO
    NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

write_basic_package_version_file(
    "${PROJECT_BINARY_DIR}/cmake/zsync2ConfigVersion.cmake"
    VERSION "${VERSION}"
    COMPATIBILITY AnyNewerVersion
)

install(FILES
    "${PROJECT_BINARY_DIR}/cmake/zsync2Config.cmake"
    "${PROJECT_BINARY_DIR}/cmake/zsync2ConfigVersion.cmake"
    DESTINATION lib/cmake/zsync2
)
