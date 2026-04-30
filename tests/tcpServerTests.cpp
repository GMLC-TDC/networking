/*
Copyright (c) 2017-2026,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance
for Sustainable Energy, LLC.  See the top-level NOTICE for additional details.
All rights reserved. SPDX-License-Identifier: BSD-3-Clause
*/

#include "catch.hpp"

#include "gmlc/networking/TcpOperations.h"
#include "gmlc/networking/addressOperations.hpp"
#include "gmlc/networking/interfaceOperations.hpp"

TEST_CASE("localHost", "[TcpServer]")
{
    asio::io_context io_context;
    auto localhost = "127.0.0.1";
    CHECK_NOTHROW(
        gmlc::networking::TcpServer::create(io_context, localhost, "0", false));
}

TEST_CASE("externalAddressV4", "[TcpServer]")
{
    asio::io_context io_context;
    CHECK_NOTHROW(
        gmlc::networking::TcpServer::create(
            io_context,
            gmlc::networking::getLocalExternalAddressV4(),
            "0",
            false));
}

TEST_CASE("localHostString", "[TcpServer]")
{
    asio::io_context io_context;
    CHECK_NOTHROW(
        gmlc::networking::TcpServer::create(
            io_context, "localhost", "0", false));
}
TEST_CASE("invalidString", "[TcpServer]")
{
    asio::io_context io_context;
    CHECK_THROWS(
        gmlc::networking::TcpServer::create(
            io_context, "testString", "0", false));
}
