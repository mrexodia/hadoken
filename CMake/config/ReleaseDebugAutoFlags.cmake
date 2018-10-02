# ReleaseDebugAutoFlags.cmake
#
# Release / Debug configuration helper
#
# License: BSD 3
#
# Copyright (c) 2016, Adrien Devresse
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.



## default configuration
if(NOT CMAKE_BUILD_TYPE AND (NOT CMAKE_CONFIGURATION_TYPES))
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the type of build." FORCE)
    message(STATUS "Setting build type to '${CMAKE_BUILD_TYPE}' as none was specified.")
endif()


# Different configuration types:
#
# Debug : Optimized for debugging, include symbols
# Release : Release mode, no debuginfo
# RelWithDebInfo : Distribution mode, basic optimizations for potable code with debuginfos
# Fast : Maximum level of optimization. Target native architecture, not portable code

include(CompilerFlagsHelpers)

## option to enable / disable hardening
option(CMAKE_ENABLE_HARDENING "Enable or disable hardening protections" TRUE)

if(CMAKE_ENABLE_HARDENING)
    set(CMAKE_C_HARDNING_FLAGS "${CMAKE_C_STACK_PROTECTION} ${CMAKE_C_FORTIFY}")
    set(CMAKE_CXX_HARDNING_FLAGS "${CMAKE_CXX_STACK_PROTECTION} ${CMAKE_CXX_FORTIFY}")
else()
    set(CMAKE_C_HARDENING_FLAGS "")
    set(CMAKE_CXX_HARDNING_FLAGS "")
endif()


## option to enable / disable address & undefined sanitizers
option(CMAKE_ENABLE_ASAN "Enable or disable address and undefined sanitizers" FALSE)

if(CMAKE_ENABLE_ASAN)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_C_ASAN}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_ASAN}")
endif()


## option to enable / disable thread sanitizer
option(CMAKE_ENABLE_TSAN "Enable or disable thread sanitizer" FALSE)

if(CMAKE_ENABLE_TSAN)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_C_TSAN}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_TSAN}")
endif()


## option to enable / disable GCOV
option(CMAKE_ENABLE_GCOV "Enable or disable gcov" FALSE)

if(CMAKE_ENABLE_GCOV)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()


## option to enable / disable clang-tidy
option(CMAKE_ENABLE_CLANG_TIDY "Enable or disable clang-tidy" FALSE)

if(CMAKE_ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_BIN NAMES clang-tidy)
    find_program(RUN_CLANG_TIDY_BIN NAMES run-clang-tidy.py run-clang-tidy-4.0.py HINTS $ENV{CLANG_ROOT}/share/clang/ /usr/share/clang/)

    list(APPEND CLANG_TIDY_ARGS
            ${RUN_CLANG_TIDY_BIN} -clang-tidy-binary ${CLANG_TIDY_BIN} -header-filter=.*/${PROJECT_SOURCE_DIR}/.*)

    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

    add_custom_target(
            clang-tidy
            COMMAND ${CLANG_TIDY_ARGS} -checks=*
            COMMENT "running clang tidy (*)")

    add_custom_target(
            clang-tidy-cert
            COMMAND ${CLANG_TIDY_ARGS} -checks=-*,cert*
            COMMENT "running clang tidy (cert)")

    add_custom_target(
            clang-tidy-misc
            COMMAND ${CLANG_TIDY_ARGS} -checks=-*,misc*
            COMMENT "running clang tidy (misc)")

    add_custom_target(
            clang-tidy-google
            COMMAND ${CLANG_TIDY_ARGS} -checks=-*,google*
            COMMENT "running clang tidy (google)")

    add_custom_target(
            clang-tidy-bugprone
            COMMAND ${CLANG_TIDY_ARGS} -checks=-*,bugprone*
            COMMENT "running clang tidy (bugprone)")

    add_custom_target(
            clang-tidy-modernize
            COMMAND ${CLANG_TIDY_ARGS} -checks=-*,modernize*
            COMMENT "running clang tidy (modernize)")

    add_custom_target(
            clang-tidy-performance
            COMMAND ${CLANG_TIDY_ARGS} -checks=-*,performance*
            COMMENT "running clang tidy (performance)")

    add_custom_target(
            clang-tidy-readability
            COMMAND ${CLANG_TIDY_ARGS} -checks=-*,readability*
            COMMENT "running clang tidy (readability)")

    add_custom_target(
            clang-tidy-clang-analyzer
            COMMAND ${CLANG_TIDY_ARGS} -checks=-*,clang-analyzer*
            COMMENT "running clang tidy (clang-analyzer)")

    add_custom_target(
            clang-tidy-cppcoreguidelines
            COMMAND ${CLANG_TIDY_ARGS} -checks=-*,cppcoreguidelines*
            COMMENT "running clang tidy (cppcoreguidelines)")
endif()


set(CMAKE_C_FLAGS_RELEASE  "${CMAKE_C_WARNING_ALL} ${CMAKE_C_OPT_NORMAL} ${CMAKE_C_DISABLE_ASSERT} ${CMAKE_C_HARDNING_FLAGS}")
set(CMAKE_C_FLAGS_DEBUG  "${CMAKE_C_DEBUGINFO_FLAGS} ${CMAKE_C_WARNING_ALL} ${CMAKE_C_OPT_NONE} ${CMAKE_C_HARDNING_FLAGS}")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_DEBUGINFO_FLAGS} ${CMAKE_C_WARNING_ALL} ${CMAKE_C_OPT_NORMAL} ${CMAKE_C_DISABLE_ASSERT} ${CMAKE_C_HARDNING_FLAGS}")
set(CMAKE_C_FLAGS_FAST "${CMAKE_C_WARNING_ALL} ${CMAKE_C_OPT_FASTEST} ${CMAKE_C_LINK_TIME_OPT} ${CMAKE_C_GEN_NATIVE} ${CMAKE_C_DISABLE_ASSERT}")



set(CMAKE_CXX_FLAGS_RELEASE  "${CMAKE_CXX_WARNING_ALL} ${CMAKE_CXX_OPT_NORMAL} ${CMAKE_CXX_DISABLE_ASSERT} ${CMAKE_CXX_HARDNING_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG  "${CMAKE_CXX_DEBUGINFO_FLAGS} ${CMAKE_CXX_WARNING_ALL} ${CMAKE_CXX_OPT_NONE} ${CMAKE_CXX_HARDNING_FLAGS}")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_DEBUGINFO_FLAGS} ${CMAKE_CXX_WARNING_ALL} ${CMAKE_CXX_OPT_NORMAL} ${CMAKE_CXX_DISABLE_ASSERT} ${CMAKE_CXX_HARDNING_FLAGS}")
set(CMAKE_CXX_FLAGS_FAST "${CMAKE_CXX_WARNING_ALL} ${CMAKE_CXX_OPT_FASTEST} ${CMAKE_CXX_LINK_TIME_OPT} ${CMAKE_CXX_GEN_NATIVE} ${CMAKE_CXX_DISABLE_ASSERT}")

