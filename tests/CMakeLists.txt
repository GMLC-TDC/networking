# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2021, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

set(NETWORKING_TESTS
    addressOperationsTests
    interfaceOperationsTests
    tcpOperationsTests
    tcpServerTests
    tcpClientTests
    contextManagerTests
    server
    client
)

foreach(T ${NETWORKING_TESTS})

    add_executable(${T} ${T}.cpp)
    target_link_libraries(${T} PUBLIC gmlc::networking)
    add_test(${T} ${T})
    set_target_properties(${T} PROPERTIES FOLDER tests)
    if(GMLC_NETWORKING_CLANG_TIDY)
        set_property(TARGET ${T} PROPERTY CXX_CLANG_TIDY "${DO_CLANG_TIDY}")
    endif()
    target_include_directories(${T} PRIVATE ${PROJECT_SOURCE_DIR}/ThirdParty/catch2)
endforeach()

if(CMAKE_BUILD_TYPE STREQUAL Coverage)
    include(CodeCoverage)
    setup_target_for_coverage(
        NAME NETWORKING_coverage EXECUTABLE ctest DEPENDENCIES ${NETWORKING_TESTS}
    )
endif()
