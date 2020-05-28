cmake_minimum_required(VERSION 3.10)
project(ttl)

add_library(ttl INTERFACE)
add_library(ttl::ttl ALIAS ttl)
target_compile_features(ttl INTERFACE cxx_std_14)
target_include_directories(ttl INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

find_package (Git)
if (GIT_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} describe --tags --dirty=-d
        OUTPUT_VARIABLE VERSION)
else (GIT_FOUND)
      set(VERSION "v0.0.0-0-00000000-d")
endif (GIT_FOUND)

#parse the version information into pieces.
string(REGEX REPLACE "^v([0-9]+)\\..*" "\\1" VERSION_MAJOR "${VERSION}")
string(REGEX REPLACE "^v[0-9]+\\.([0-9]+).*" "\\1" VERSION_MINOR "${VERSION}")
string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" VERSION_PATCH "${VERSION}")
string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.[0-9]+-([0-9]+).*" "\\1" VERSION_COMMIT "${VERSION}")
string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.[0-9]+-[0-9]+-(.*)" "\\1" VERSION_SHA1 "${VERSION}")
set(VERSION_SHORT "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/version.cpp.in
#                ${CMAKE_CURRENT_BINARY_DIR}/version.cpp)
#set(version_file "${CMAKE_CURRENT_BINARY_DIR}/version.cpp")

# Testing
enable_testing()
include(GoogleTest)

find_package(ZLIB REQUIRED)

set(TEST_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/AnyTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/BinaryReaderWriterTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ConfigTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ContractTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CRCTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/DeflaterTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/DynLibTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InflaterTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/LINQTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/LoggerTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/MMAPTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ModuleTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PredicateTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ProcessTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/RCUTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SignalTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/StringTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/TimerTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/TypeTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ZipReaderTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ZipStreamTest.cpp)

add_executable(ttl-test ${TEST_SOURCES})
target_compile_options(ttl-test PRIVATE 
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:Clang>:-Weverything>)
target_link_libraries(ttl-test PRIVATE ttl gtest gtest_main pthread ZLIB::ZLIB ${CMAKE_DL_LIBS})

gtest_add_tests(TARGET ttl-test)

install(
    DIRECTORY ${CMAKE_SOURCE_DIR}/include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h*")

set(CPACK_PACKAGE_NAME "libttl")
set(CPACK_PACKAGE_VENDOR "Dominik Thalhammer")
set(CPACK_PACKAGE_CONTACT "dominik@thalhammer.it")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Tiny Template Library")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/Thalhammer/ttl")
set(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/../LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/../README.md")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Dominik Thalhammer <dominik@thalhammer.it>")
set(CPACK_DEBIAN_PACKAGE_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}-${VERSION_COMMIT}")
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)

include(CPack)