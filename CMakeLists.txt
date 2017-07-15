# pimp of https://rix0r.nl/blog/2015/08/13/cmake-guide/
# Define library. Only source files here!
#project(liblib VERSION 0.1 LANGUAGES CXX)

#set(CMAKE_BUILD_TYPE Release)

cmake_minimum_required(VERSION 2.6)

include_directories(${CMAKE_SOURCE_DIR})

#
## Extra build option
#
#add_definitions(-DVERBOSE)
add_definitions(-DWITHOUT_PCRE)

#
## LLVM area
#

SET (CMAKE_AR      "/usr/bin/llvm-ar")
SET (CMAKE_LINKER  "/usr/bin/llvm-ld")
SET (CMAKE_NM      "/usr/bin/llvm-nm")
SET (CMAKE_OBJDUMP "/usr/bin/llvm-objdump")
SET (CMAKE_RANLIB  "/usr/bin/llvm-ranlib")

#set(CMAKE_C_COMPILER "/usr/bin/clang")
#set(CMAKE_CXX_COMPILER "/usr/bin/clang++")

#add_definitions(-O0)
#add_definitions(-emit-llvm)
#SET (CMAKE_LINKER  "/usr/bin/llvm-link")
#add_definitions(-lrt)

#list(APPEND LIBRARIES "pth")
#add_definitions(-DUSE_GREEN_THREAD)
#add_definitions(-DPTH_DEBUG)


# sources and doc used to create functions:
#	function: https://cmake.org/cmake/help/v3.0/command/function.html
#	set(SOURCES ... PARENT_SCOPE): http://stackoverflow.com/questions/10031953/how-to-set-the-global-variable-in-a-function-for-cmake
#	for debug: https://cmake.org/cmake/help/v3.0/command/message.html


#http://stackoverflow.com/questions/33222569/best-way-to-portably-set-compiler-options-like-wall-and-pedantic-in-cmake

include(CheckCXXCompilerFlag)

function(enable_cxx_compiler_flag_if_supported flag)
    string(FIND "${CMAKE_CXX_FLAGS}" "${flag}" flag_already_set)
    if(flag_already_set EQUAL -1)
        check_cxx_compiler_flag("${flag}" flag_supported)
        if(flag_supported)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}" PARENT_SCOPE)
            set(CXX_FLAGS "${CXX_FLAGS} ${flag}" PARENT_SCOPE)
            set(CXXFLAGS "${CXXFLAGS} ${flag}" PARENT_SCOPE)
		list(APPEND CXXFLAGS "${flag}")
		add_definitions("${flag}")
        endif()
    endif()
endfunction()

#enable_cxx_compiler_flag_if_supported("-Wall")
#enable_cxx_compiler_flag_if_supported("-Wextra")
#enable_cxx_compiler_flag_if_supported("-pedantic")

###### function for embedded library compilation ######
function(lxc_build_add_embedded_library name)
	string(TOUPPER ${name} LIB_NAME_UPPER)
	string(TOLOWER ${name} LIB_NAME_LOWER)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DLXC_EMBED_MODULE_${LIB_NAME_UPPER}")
	SET(CXXFLAGS "${CXXFLAGS} -DLXC_EMBED_MODULE_${LIB_NAME_UPPER}")
	add_definitions(-DLXC_EMBED_MODULE_${LIB_NAME_UPPER})
	file(GLOB_RECURSE CRNT_SOURCES "" "${LIB_NAME_LOWER}/*.c")
	set(SOURCES ${SOURCES} ${CRNT_SOURCES} PARENT_SCOPE)
endfunction(lxc_build_add_embedded_library)

###### function for embedded test modules ######
function(lxc_build_add_embedded_test name)
	string(TOLOWER ${name} LIB_NAME_LOWER)
	file(GLOB_RECURSE CRNT_SOURCES "" "${LIB_NAME_LOWER}/*.c")
	set(SOURCES ${SOURCES} ${CRNT_SOURCES} PARENT_SCOPE)
endfunction(lxc_build_add_embedded_test)

######  add novaprova to the target compilation ######
# sources and docs: 
#	novapropva: http://novaprova.readthedocs.io/en/latest/building.html
#	pkg-config: http://stackoverflow.com/questions/32507252/how-to-include-pkg-config-cflags-libs-gtk-2-0-in-cxx-flags-of-cmake
#	
function(include_novaprova)
	find_package(PkgConfig)
	if (PKG_CONFIG_FOUND)
		pkg_check_modules(NOVAPROVA "novaprova")
		if (NOVAPROVA_FOUND)
			set(LIBRARIES ${LIBRARIES} ${NOVAPROVA_LIBRARIES} PARENT_SCOPE)
			add_definitions(${NOVAPROVA_CFLAGS} ${NOVAPROVA_CFLAGS_OTHER} -gdwarf -DINCLUDE_NOVAPROVA)
			SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DINCLUDE_NOVAPROVA")
			SET(CXXFLAGS "${CXXFLAGS} -DINCLUDE_NOVAPROVA")
		endif()
	endif()
endfunction(include_novaprova)


#arithmetic library
lxc_build_add_embedded_library(arithmetic)

#posix library
#lxc_build_add_embedded_library(posix)

#tests
lxc_build_add_embedded_test(test-core)
lxc_build_add_embedded_library(experiment)
#lxc_build_add_embedded_library(hideout)#
include_novaprova()


###### core library ######
file(GLOB_RECURSE CORE_SOURCES "" "core/*.c")
file(GLOB_RECURSE JUNKS ${CMAKE_SOURCE_DIR} "CMakeCCompilerId.c")

foreach(JUNK ${JUNKS})
	list(REMOVE_ITEM CORE_SOURCES ${JUNK})
endforeach(JUNK)

list(APPEND SOURCES ${CORE_SOURCES})

###### function for shared library ######
function(output_shared)
	set(CMAKE_POSITION_INDEPENDENT_CODE ON)
	add_library(logxcontroll-core SHARED ${SOURCES})
endfunction(output_shared)

function(output_executable)
	add_executable(logxcontroll-core ${SOURCES})
endfunction(output_executable)

#output_shared()
output_executable()

###### find and include external libraries
#	sources and docs:
#		http://stackoverflow.com/questions/1620918/cmake-and-libpthread
#		http://stackoverflow.com/questions/33678965/need-to-link-cmake-project-to-dl-library		
#
find_package (Threads REQUIRED)
list(APPEND LIBRARIES ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS})

find_package (PCRE REQUIRED)
list(APPEND LIBRARIES ${PCRE_LIBRARIES})


set(CMAKE_XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT "dwarf-with-dsym")

target_link_libraries (logxcontroll-core ${LIBRARIES})
set_property(TARGET logxcontroll-core PROPERTY C_STANDARD 99)

#
# DEBUG info
#


message(STATUS "target_link_libraries: " "${LIBRARIES}")
get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
message(STATUS "Included dirs: " "${dirs}")
message(STATUS "Link_Options: " "${LIBRARIES}")
message(STATUS "Source_Files: " "${SOURCES}")
#https://stackoverflow.com/questions/5403705/cmake-add-definitions-and-compile-definitions-how-to-see-them
get_directory_property(DirDefs DIRECTORY ${CMAKE_SOURCE_DIR} COMPILE_DEFINITIONS)
message(STATUS "Compile_Definitions: " "${DirDefs}")
#message(STATUS ":" "${}")
