# pimp of https://rix0r.nl/blog/2015/08/13/cmake-guide/
# Define library. Only source files here!
#project(liblib VERSION 0.1 LANGUAGES CXX)

#set(CMAKE_BUILD_TYPE Release)

cmake_minimum_required(VERSION 3.0)

include_directories(${CMAKE_SOURCE_DIR}/../)

file(GLOB_RECURSE CORE_SOURCES ${CMAKE_SOURCE_DIR} "*.c")

add_library(logxcontroll-posix SHARED ${CORE_SOURCES})

# Define headers for this library. PUBLIC headers are used for
# compiling the library, and will be added to consumers' build
# paths.
#target_include_directories(lib PUBLIC
#    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
#    $<INSTALL_INTERFACE:include>
#    PRIVATE src)

# This makes the project importable from the install directory
# Put config file in per-project dir (name MUST match), can also
# just go into <prefix>/cmake.
#install(EXPORT MyLibraryConfig DESTINATION share/MyLibrary/cmake)

# This makes the project importable from the build directory
#export(TARGETS lib FILE MyLibraryConfig.cmake)

# Every library has unit tests, of course # i obey of course
#add_executable(testlib
#    test/testlib.cpp)

#target_link_libraries(testlib
#    lib)

#add_test(testlib testlib)
