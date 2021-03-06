
if(HADOKEN_UNIT_TESTS)

option(WITH_SANITIZER_ALL "Enable sanitizer on tests" TRUE)

if(WITH_SANITIZER_ALL)
    list(APPEND SANITIZER_FLAGS "-fno-sanitize-recover" "-fsanitize=undefined")
endif()

find_package(Boost 1.41.0 QUIET REQUIRED system unit_test_framework)

include_directories(${CMAKE_SOURCE_DIR})
include_directories(SYSTEM ${Boost_INCLUDE_DIRS} )

link_directories( ${Boost_LIBRARY_DIRS} )

if(NOT Boost_USE_STATIC_LIBS)
        add_definitions(-DBOOST_TEST_DYN_LINK=TRUE)
endif()



## generic Test
LIST(APPEND test_generic_src "test_generic.cpp")

add_executable(test_generic ${test_generic_src} ${HADOKEN_HEADERS} ${HADOKEN_HEADERS_1})
target_link_libraries(test_generic ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES} ${SANITIZER_FLAGS})
target_compile_options(test_generic PRIVATE ${SANITIZER_FLAGS})

add_test(NAME test_generic_unit COMMAND ${TESTS_PREFIX} ${TESTS_PREFIX_ARGS} ${CMAKE_CURRENT_BINARY_DIR}/test_generic)


## random generator related tests
LIST(APPEND test_random_src "test_random.cpp")

add_executable(test_random ${test_random_src} ${HADOKEN_HEADERS} ${HADOKEN_HEADERS_1})
target_link_libraries(test_random ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES} ${SANITIZER_FLAGS})
target_compile_options(test_random PRIVATE ${SANITIZER_FLAGS})

add_test(NAME test_random_unit COMMAND ${TESTS_PREFIX} ${TESTS_PREFIX_ARGS} ${CMAKE_CURRENT_BINARY_DIR}/test_random)


## crypto generator related tests
LIST(APPEND test_crypto_src "test_crypto.cpp")

add_executable(test_crypto ${test_crypto_src} ${HADOKEN_HEADERS} ${HADOKEN_HEADERS_1})
target_link_libraries(test_crypto ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES} ${SANITIZER_FLAGS})
target_compile_options(test_crypto PRIVATE ${SANITIZER_FLAGS})

add_test(NAME test_crypto_unit COMMAND ${TESTS_PREFIX} ${TESTS_PREFIX_ARGS} ${CMAKE_CURRENT_BINARY_DIR}/test_crypto)



## format related tests
LIST(APPEND test_format_src "test_format.cpp")

add_executable(test_format ${test_format_src} ${HADOKEN_HEADERS} ${HADOKEN_HEADERS_1})
target_link_libraries(test_format ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES} ${SANITIZER_FLAGS})
target_compile_options(test_format PRIVATE ${SANITIZER_FLAGS})

add_test(NAME test_format_unit COMMAND ${TESTS_PREFIX} ${TESTS_PREFIX_ARGS} ${CMAKE_CURRENT_BINARY_DIR}/test_format)


## containers Test
LIST(APPEND test_container_src "test_container.cpp")

add_executable(test_container ${test_container_src} ${HADOKEN_HEADERS} ${HADOKEN_HEADERS_1})
target_link_libraries(test_container ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${SANITIZER_FLAGS})
target_compile_options(test_container PRIVATE ${SANITIZER_FLAGS})

add_test(NAME test_container_unit COMMAND ${TESTS_PREFIX} ${TESTS_PREFIX_ARGS} ${CMAKE_CURRENT_BINARY_DIR}/test_container)


## string Test
LIST(APPEND test_string_src "test_string.cpp")

add_executable(test_string ${test_string_src} ${HADOKEN_HEADERS} ${HADOKEN_HEADERS_1})
target_link_libraries(test_string ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES} ${SANITIZER_FLAGS})
target_compile_options(test_string PRIVATE ${SANITIZER_FLAGS})

add_test(NAME test_string_unit COMMAND ${TESTS_PREFIX} ${TESTS_PREFIX_ARGS} ${CMAKE_CURRENT_BINARY_DIR}/test_string)


## os Test
LIST(APPEND test_os_src "test_os.cpp")

add_executable(test_os ${test_os_src} ${HADOKEN_HEADERS} ${HADOKEN_HEADERS_1})
target_link_libraries(test_os ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES} ${SANITIZER_FLAGS})
target_compile_options(test_os PRIVATE ${SANITIZER_FLAGS})

add_test(NAME test_os_unit COMMAND ${TESTS_PREFIX} ${TESTS_PREFIX_ARGS} ${CMAKE_CURRENT_BINARY_DIR}/test_os)


## thread Test
LIST(APPEND test_thread_src "test_thread.cpp")

add_executable(test_thread ${test_thread_src} ${HADOKEN_HEADERS} ${HADOKEN_HEADERS_1})
target_link_libraries(test_thread ${CMAKE_THREAD_LIBS_INIT} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES}  ${SANITIZER_FLAGS})
target_compile_options(test_thread PRIVATE ${SANITIZER_FLAGS})

add_test(NAME test_thread_unit COMMAND ${TESTS_PREFIX} ${TESTS_PREFIX_ARGS} ${CMAKE_CURRENT_BINARY_DIR}/test_thread)



## Parallel Test
LIST(APPEND test_parallel_src "test_parallel.cpp")

add_executable(test_parallel_base ${test_parallel_src} ${HADOKEN_HEADERS} ${HADOKEN_HEADERS_1})
target_link_libraries(test_parallel_base ${CMAKE_THREAD_LIBS_INIT} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES}  ${SANITIZER_FLAGS})
target_compile_options(test_parallel_base PRIVATE ${SANITIZER_FLAGS})

add_test(NAME test_parallel_base_unit COMMAND ${TESTS_PREFIX} ${TESTS_PREFIX_ARGS} ${CMAKE_CURRENT_BINARY_DIR}/test_parallel_base)



# Network Test
LIST(APPEND test_network_src "test_network.cpp")

add_executable(test_network_base ${test_network_src} ${HADOKEN_HEADERS} ${HADOKEN_HEADERS_1})
target_link_libraries(test_network_base ${CMAKE_THREAD_LIBS_INIT} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES} ${SANITIZER_FLAGS})
target_compile_options(test_network_base PRIVATE ${SANITIZER_FLAGS})

add_test(NAME test_network_base_unit COMMAND ${TESTS_PREFIX} ${TESTS_PREFIX_ARGS} ${CMAKE_CURRENT_BINARY_DIR}/test_network_base)

# fsm Test
LIST(APPEND test_fsm_src "test_fsm.cpp")

add_executable(test_fsm_base ${test_fsm_src} ${HADOKEN_HEADERS} ${HADOKEN_HEADERS_1})
target_link_libraries(test_fsm_base ${CMAKE_THREAD_LIBS_INIT} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES} ${SANITIZER_FLAGS})
target_compile_options(test_fsm_base PRIVATE ${SANITIZER_FLAGS})

add_test(NAME test_fsm_base_unit COMMAND ${TESTS_PREFIX} ${TESTS_PREFIX_ARGS} ${CMAKE_CURRENT_BINARY_DIR}/test_fsm_base)


# opt_parsers Test
LIST(APPEND test_opts_parser_src "test_opts_parser.cpp")

add_executable(test_opts_parser ${test_opts_parser_src} ${HADOKEN_HEADERS} ${HADOKEN_HEADERS_1})
target_link_libraries(test_opts_parser ${CMAKE_THREAD_LIBS_INIT} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES} ${SANITIZER_FLAGS})
target_compile_options(test_fsm_base PRIVATE ${SANITIZER_FLAGS})

add_test(NAME test_opts_parser_uint COMMAND ${TESTS_PREFIX} ${TESTS_PREFIX_ARGS} ${CMAKE_CURRENT_BINARY_DIR}/test_opts_parser)


endif()
