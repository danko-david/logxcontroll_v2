# https://rix0r.nl/blog/2015/08/13/cmake-guide/
# Define library. Only source files here!
cmake_minimum_required(VERSION 2.6)

#FILE GLOB_RECURSE calls should not follow symlinks.
if(POLICY CMP0009)
	cmake_policy(SET CMP0009 NEW)
endif()

############################ Load config file #################################
#
option(BUILD_CONFIG "BUILD_CONFIG" OFF)

if(NOT BUILD_CONFIG)
	message(FATAL_ERROR "BUILD_CONFIG file not specified, call: cmake -DBUILD_CONFIG=buildconfigs/...")
endif()

if(NOT EXISTS ${BUILD_CONFIG})
	message(FATAL_ERROR "BUILD_CONFIG=${BUILD_CONFIG} file doesn't exists.")
endif()


#load config file
FILE(READ "${BUILD_CONFIG}" BUILD_CONFIG_FILE_CONTENT)
string(REPLACE "\n" ";" BUILD_CONFIG_FILE_CONTENT ${BUILD_CONFIG_FILE_CONTENT})

message(STATUS "build_cfg: " "${BUILD_CONFIG}")

#drop empty lines and lines starts with #
# https://cmake.org/pipermail/cmake/2006-October/011734.html
#
foreach(bcl ${BUILD_CONFIG_FILE_CONTENT})
	STRING(STRIP ${bcl}, bcl)
	STRING(REGEX MATCH "^[-_A-Z]+" add ${bcl})
	IF(add)
		LIST(APPEND BUILD_CONFIG_CONTENT ${add})
	ENDIF(add)
endforeach()

message(STATUS "cfg_content: " "${BUILD_CONFIG_CONTENT}")


########################### build options helper function #####################
# https://stackoverflow.com/questions/23323147/best-way-to-check-with-cmake-whether-list-containts-a-specific-entry
#
#
function(cfg_isset_and_use opt)
	list (FIND BUILD_CONFIG_CONTENT "${opt}" _index)
	if (${_index} GREATER -1)
		set(C_OPT TRUE PARENT_SCOPE)
		set(BUILD_USED_OPTIONS ${BUILD_USED_OPTIONS} ${opt} PARENT_SCOPE)		
		return()
	endif()
	set(C_OPT OFF PARENT_SCOPE)
endfunction(cfg_isset_and_use)

# https://cmake.org/pipermail/cmake/2011-March/043128.html
# refuse Makefile creation if incompatible build options are set
function(cfg_check_incompatible)
	foreach(f ${ARGV})
		list (FIND BUILD_CONFIG_CONTENT "${f}" _index)
		if (${_index} GREATER -1)
			if(prev_val)
				message(FATAL_ERROR "`${prev_val}` and `${f}` build options are incompatible")
			else()
				set(prev_val "${f}")
			endif()
		endif()
	endforeach()
endfunction(cfg_check_incompatible)

#
## LogxControll build helper functions
#
# sources and doc used to create functions:
#	function: https://cmake.org/cmake/help/v3.0/command/function.html
#	set(SOURCES ... PARENT_SCOPE): http://stackoverflow.com/questions/10031953/how-to-set-the-global-variable-in-a-function-for-cmake
#	for debug: https://cmake.org/cmake/help/v3.0/command/message.html
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

###### function for shared library ######
function(output_shared)
	set(CMAKE_POSITION_INDEPENDENT_CODE ON)
	add_library(logxcontroll-core SHARED ${SOURCES})
endfunction(output_shared)

function(output_executable)
	add_executable(logxcontroll-core ${SOURCES})
endfunction(output_executable)

#
## Other functions
#
#http://stackoverflow.com/questions/33222569/best-way-to-portably-set-compiler-options-like-wall-and-pedantic-in-cmake
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


################################# Review this #################################
#
## LLVM area
#

#SET (CMAKE_AR      "/usr/bin/llvm-ar")
#SET (CMAKE_LINKER  "/usr/bin/llvm-ld")
#SET (CMAKE_LINKER  "/usr/bin/llvm-ld")
#SET (CMAKE_NM      "/usr/bin/llvm-nm")
#SET (CMAKE_OBJDUMP "/usr/bin/llvm-objdump")
#SET (CMAKE_RANLIB  "/usr/bin/llvm-ranlib")

#set(CMAKE_C_COMPILER "/usr/bin/clang")
#set(CMAKE_CXX_COMPILER "/usr/bin/clang++")

#add_definitions(-O0)
#add_definitions(-emit-llvm)
#SET (CMAKE_LINKER  "/usr/bin/llvm-link")
#add_definitions(-lrt)


###################### Options required on all build type #####################

include_directories(${CMAKE_SOURCE_DIR})
include(CheckCXXCompilerFlag)


#
## Extra build options
#

#
## Verbose build (preprocessor option)
#
cfg_isset_and_use(VERBOSE_BUILD)
if(C_OPT)
	add_definitions(-DVERBOSE_BUILD)
endif()

#
## Disable PCRE
# TODO it's a temporary solution
cfg_isset_and_use(WITHOUT_PCRE)
if(C_OPT)
	add_definitions(-DWITHOUT_PCRE)
else()
	find_package (PCRE REQUIRED)
	list(APPEND LIBRARIES ${PCRE_LIBRARIES})
endif()

#
## Green threads vs OS threads
#
cfg_isset_and_use(USE_GREEN_THREAD)
if(C_OPT)
	add_definitions(-DUSE_GREEN_THREAD)
	list(APPEND LIBRARIES "pth")
else()
	find_package (Threads REQUIRED)
	list(APPEND LIBRARIES ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS})
endif()

#
## Pedantic compilation
#
cfg_isset_and_use(PEDANTIC_BUILD)
if(C_OPT)
	enable_cxx_compiler_flag_if_supported("-Wall")
	enable_cxx_compiler_flag_if_supported("-Wextra")
	enable_cxx_compiler_flag_if_supported("-pedantic")
endif()


#arithmetic library
cfg_isset_and_use(LXC_EMBED_MODULE_ARITHMETIC)
if(C_OPT)
	lxc_build_add_embedded_library(arithmetic)	
endif()

#posix library
cfg_isset_and_use(LXC_EMBED_MODULE_POSIX)
if(C_OPT)
	lxc_build_add_embedded_library(posix)
endif()


cfg_isset_and_use(LXC_TEST_CORE)
if(C_OPT)
	lxc_build_add_embedded_test(test-core)
endif()


cfg_isset_and_use(LXC_EMBED_MODULE_EXPERIMENT)
if(C_OPT)
	lxc_build_add_embedded_library(experiment)
endif()


cfg_isset_and_use(LXC_EMBED_MODULE_HIDEOUT)
if(C_OPT)
	lxc_build_add_embedded_library(hideout)
endif()


cfg_isset_and_use(INCLUDE_NOVAPROVA)
if(C_OPT)
	include_novaprova()
	#TODO require debug mode
endif()



###### core library ######
file(GLOB_RECURSE CORE_SOURCES "" "core/*.c")
file(GLOB_RECURSE JUNKS ${CMAKE_SOURCE_DIR} "CMakeCCompilerId.c")
foreach(JUNK ${JUNKS})
	list(REMOVE_ITEM CORE_SOURCES ${JUNK})
endforeach(JUNK)
list(APPEND SOURCES ${CORE_SOURCES})




cfg_check_incompatible(BUILD_DEBUG BUILD_RELEASE)
cfg_isset_and_use(BUILD_RELEASE) # side effect: mark as used
cfg_isset_and_use(BUILD_DEBUG)
if(C_OPT)
	set(CMAKE_BUILD_TYPE Debug)
	set(CMAKE_XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT "dwarf-with-dsym")
else()
	set(CMAKE_BUILD_TYPE Release)
	#TODO add_definitions(-flto)#-x lto
endif()



cfg_check_incompatible(OUTPUT_EXECUTABLE OUTPUT_SHARED)
cfg_isset_and_use(OUTPUT_EXECUTABLE)# side effect: mark as used option
cfg_isset_and_use(OUTPUT_SHARED)
if(C_OPT)
	output_shared()
else()
	output_executable()
endif()


###### find and include external libraries
#	sources and docs:
#		http://stackoverflow.com/questions/1620918/cmake-and-libpthread
#		http://stackoverflow.com/questions/33678965/need-to-link-cmake-project-to-dl-library		
#

target_link_libraries (logxcontroll-core ${LIBRARIES})
set_property(TARGET logxcontroll-core PROPERTY C_STANDARD 99)

#TODO check  add _D_ definitions and look up for unused Build options, refuse Makefile creation

#
## add custom _D_ definitions
#
foreach(cnt ${BUILD_CONFIG_CONTENT})
	STRING(REGEX MATCH "^_D_.*" cnt ${cnt})
	IF(cnt)
		STRING(REGEX REPLACE "^_D_(.*)" "\\1" add ${cnt})
		IF(add)
			#mark as used
			cfg_isset_and_use(add)
			add_definitions("-D${add}")
		ENDIF(add)
	ENDIF(cnt)
endforeach()


foreach(opt ${BUILD_CONFIG_CONTENT})
	list(FIND BUILD_USED_OPTIONS "${opt}" _index)
	if (${_index} LESS 0)
		 message(FATAL_ERROR "Unrecognised build option: ${opt}")
	endif()
endforeach()

#enable_testing()

#
# DEBUG info (Don't remove/comment out, tools/cmake/code_all_in_one.sh utility uses this output)
#

message(STATUS "build_config_file: " "${BUILD_CONFIG}")
message(STATUS "target_link_libraries: " "${LIBRARIES}")
get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
message(STATUS "Included dirs: " "${dirs}")
message(STATUS "Link_Options: " "${LIBRARIES}")
message(STATUS "Source_Files: " "${SOURCES}")
#https://stackoverflow.com/questions/5403705/cmake-add-definitions-and-compile-definitions-how-to-see-them
#get_directory_property(DirDefs DIRECTORY ${CMAKE_SOURCE_DIR} COMPILE_DEFINITIONS)
get_directory_property(DirDefs COMPILE_DEFINITIONS)
message(STATUS "Compile_Definitions: " "${DirDefs}")
#message(STATUS ":" "${}")
