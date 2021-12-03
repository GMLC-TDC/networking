# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2019, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

set(GOOGLE_TEST_INDIVIDUAL OFF)
include(AddGoogletest)

set(NETWORKING_TESTS
    addressOperationsTests
)

# Only affects current directory, so safe
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

foreach(T ${NETWORKING_TESTS})

    add_executable(${T} ${T}.cpp)
    target_link_libraries(${T} PUBLIC gmlc_networking)
    add_gtest(${T})
    set_target_properties(${T} PROPERTIES FOLDER tests)
    if(GMLC_NETWORKING_CLANG_TIDY)
        set_property(TARGET ${T} PROPERTY CXX_CLANG_TIDY "${DO_CLANG_TIDY}")
    endif()
endforeach()

if(CMAKE_BUILD_TYPE STREQUAL Coverage)
    include(CodeCoverage)
    setup_target_for_coverage(
        NAME NETWORKING_coverage EXECUTABLE ctest DEPENDENCIES ${NETWORKING_TESTS}
    )
endif()
