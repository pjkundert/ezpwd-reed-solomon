# 
# ezpwd-reed-solomon -- header-only Reed-Solomon (and related) C++ library
# 
# This is a header-only library.  It installs the Reed-Solomon core headers and
# provides a CMake INTERFACE target (ezpwd::ezpwd) so downstream projects can
# either find_package(ezpwd) or consume the headers directly.
#
#     find_package(ezpwd REQUIRED)
#     target_link_libraries(myapp PRIVATE ezpwd::ezpwd)
#
# The library requires C++17.  (The legacy "definitions" precompiled table
# headers are optional under C++17 -- the static constexpr tables are
# implicitly inline -- but are still installed for backward compatibility.)
#
cmake_minimum_required(VERSION 3.14)

# Read the release version from the VERSION file (e.g. "2.2.0").
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" EZPWD_VERSION_RAW)
string(STRIP "${EZPWD_VERSION_RAW}" EZPWD_VERSION)

project(ezpwd-reed-solomon
        VERSION     ${EZPWD_VERSION}
        DESCRIPTION "Reed-Solomon error correction codecs, header-only C++"
        LANGUAGES   CXX)

# 
# The header-only INTERFACE library.
# 
add_library(ezpwd INTERFACE)
add_library(ezpwd::ezpwd ALIAS ezpwd)

# The public headers live in c++/ezpwd/ and are included as <ezpwd/...>,
# so the include root is c++/ in the build tree and include/ when installed.
target_include_directories(ezpwd INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/c++>"
    "$<INSTALL_INTERFACE:include>")

# The RS core is C++17.  (ezcod/units -- not part of this package -- need C++20
# on some toolchains because of std::numbers.)
target_compile_features(ezpwd INTERFACE cxx_std_17)

# 
# Install the Reed-Solomon core headers (and the small self-contained
# utilities they rely on) into <prefix>/include/ezpwd/.
# 
set(EZPWD_HEADERS
    rs
    rs_base
    serialize
    corrector
    output
    definitions
    rs_definitions
    serialize_definitions
    timeofday
    asserter)

include(GNUInstallDirs)

set(_ezpwd_header_files)
foreach(_h IN LISTS EZPWD_HEADERS)
    list(APPEND _ezpwd_header_files "${CMAKE_CURRENT_SOURCE_DIR}/c++/ezpwd/${_h}")
endforeach()

install(FILES ${_ezpwd_header_files}
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/ezpwd")

install(TARGETS ezpwd
        EXPORT  ezpwdTargets)

# 
# CMake package config so downstream can find_package(ezpwd).
# 
include(CMakePackageConfigHelpers)

install(EXPORT ezpwdTargets
        FILE        ezpwdTargets.cmake
        NAMESPACE   ezpwd::
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/ezpwd")

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/ezpwdConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/ezpwdConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/ezpwd")

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/ezpwdConfigVersion.cmake"
    VERSION         ${EZPWD_VERSION}
    COMPATIBILITY   AnyNewerVersion)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/ezpwdConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/ezpwdConfigVersion.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/ezpwd")

# 
# Optional test-suite (built when this is the master project, or when
# EZPWD_BUILD_TESTS is forced ON -- e.g. by the vcpkg port).
# 
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(EZPWD_MASTER_PROJECT ON)
else()
    set(EZPWD_MASTER_PROJECT OFF)
endif()
option(EZPWD_BUILD_TESTS "Build the ezpwd Reed-Solomon test executables"
       "${EZPWD_MASTER_PROJECT}")

if(EZPWD_BUILD_TESTS)
    enable_testing()

    set(EZPWD_TESTS
        rsexample
        rssimple
        rsencode
        rsembedded
        rsexercise)

    foreach(_t IN LISTS EZPWD_TESTS)
        add_executable(${_t} ${_t}.C)
        target_link_libraries(${_t} PRIVATE ezpwd::ezpwd)
        add_test(NAME ${_t} COMMAND ${_t})
    endforeach()

    # rsencode round-trip: encode -> flip one byte -> decode -> compare.
    # Implemented as a portable CMake script (no shell/perl dependency).
    add_test(NAME rsencode_roundtrip
             COMMAND ${CMAKE_COMMAND}
                     -DEZPWD_BIN=$<TARGET_FILE:rsencode>
                     -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/ezpwd-rsencode-roundtrip.cmake")
endif()
