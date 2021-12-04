# ~~~
# Copyright (c) 2017-2019, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~

set(networking_source_files AsioContextManager.cpp TcpOperations.cpp TcpConnection.cpp
                            TcpServer.cpp TcpAcceptor.cpp addressOperations.cpp
)

set(networking_header_files
    TcpOperations.h
    AsioContextManager.h
    TcpHelperClasses.h
    GuardedTypes.hpp
    TcpConnection.h
    TcpServer.h
    TcpAcceptor.h
    addressOperations.hpp
)

if(GMLC_NETWORKING_OBJECT_LIB)
    add_library(
        gmlc_networking_obj OBJECT ${networking_source_files}
                                   ${networking_header_files}
    )
    target_include_directories(
        gmlc_networking_obj PUBLIC $<BUILD_INTERFACE:${GMLC_UTILITIES_SOURCE_DIR}>
    )
    target_include_directories(
        gmlc_networking_obj
        PRIVATE $<TARGET_PROPERTY:networking_base,INTERFACE_INCLUDE_DIRECTORIES>
    )

    target_include_directories(
        gmlc_networking_obj SYSTEM
        PRIVATE $<TARGET_PROPERTY:networking_base,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
    )

    target_compile_definitions(
        gmlc_networking_obj
        PRIVATE $<TARGET_PROPERTY:networking_base,INTERFACE_COMPILE_DEFINITIONS>
    )
endif()

if(GMLC_NETWORKING_STATIC_LIB)

    add_library(
        gmlc_networking STATIC ${networking_source_files} ${networking_header_files}
                               ${string_comparison_files}
    )
    target_include_directories(
        gmlc_networking PUBLIC $<BUILD_INTERFACE:${GMLC_NETWORKING_SOURCE_DIR}>
    )

    add_library(gmlc::networking ALIAS gmlc_networking)

    target_link_libraries(gmlc_networking networking_base)
    if(GMLC_NETWORKING_INSTALL)
        install(
            TARGETS gmlc_networking ${GMLC_NETWORKING_LIBRARY_EXPORT_COMMAND}
            RUNTIME DESTINATION bin
            ARCHIVE DESTINATION lib
            LIBRARY DESTINATION lib
        )

        install(FILES ${networking_header_files} DESTINATION include)

        if(GMLC_NETWORKING_CLANG_TIDY)
            set_property(
                TARGET gmlc_networking PROPERTY CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
            )
        endif()
    endif()

endif()
