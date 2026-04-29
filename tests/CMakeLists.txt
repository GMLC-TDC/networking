# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2021, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

find_package(Catch2 CONFIG QUIET)

if(Catch2_FOUND)
    if(NOT TARGET Catch2::Catch2)
        message(FATAL_ERROR "Found Catch2 at ${Catch2_DIR} but targets are missing.")
    endif()
    message(STATUS "Found Catch2 ${Catch2_VERSION}")

    if(Catch2_VERSION VERSION_LESS 3)
        add_library(catch_main main.cpp catch.hpp)
        target_include_directories(catch_main PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
        target_link_libraries(catch_main PUBLIC Catch2::Catch2)
    else()
        add_library(catch_main ALIAS Catch2::Catch2WithMain)
        target_compile_definitions(
            Catch2::Catch2WithMain INTERFACE -DGMLC_NETWORKING_CATCH3
        )
    endif()
else()
    add_library(catch_main main.cpp catch.hpp)
    target_include_directories(catch_main PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
    target_include_directories(
        catch_main SYSTEM PUBLIC ${PROJECT_SOURCE_DIR}/ThirdParty/catch2
    )
endif()

set(NETWORKING_TESTS addressOperationsTests interfaceOperationsTests)

if(NOT GMLC_NETWORKING_DISABLE_ASIO)
    list(
        APPEND
        NETWORKING_TESTS
        simpleConnectionTests
        tcpOperationsTests
        tcpServerTests
        tcpClientTests
        contextManagerTests
    )
endif()

if(GMLC_NETWORKING_ENABLE_ENCRYPTION)
    configure_file(
        "test_files/ssl_encryption_config.json.in"
        "test_files/ssl_encryption_config.json"
    )
endif()

foreach(T ${NETWORKING_TESTS})

    add_executable(${T} ${T}.cpp)
    target_link_libraries(${T} PUBLIC gmlc::networking catch_main)
    add_test(${T} ${T})
    set_target_properties(${T} PROPERTIES FOLDER tests)
    if(GMLC_NETWORKING_CLANG_TIDY)
        set_property(TARGET ${T} PROPERTY CXX_CLANG_TIDY "${DO_CLANG_TIDY}")
    endif()
    target_compile_definitions(
        ${T} PRIVATE "TEST_BINDIR=\"${CMAKE_CURRENT_BINARY_DIR}\""
    )
    target_include_directories(${T} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
endforeach()

if(CMAKE_BUILD_TYPE STREQUAL Coverage)
    include(CodeCoverage)
    setup_target_for_coverage(
        NAME NETWORKING_coverage EXECUTABLE ctest DEPENDENCIES ${NETWORKING_TESTS}
    )
endif()
