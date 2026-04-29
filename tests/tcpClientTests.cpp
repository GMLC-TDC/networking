/*
Copyright (c) 2017-2022,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance
for Sustainable Energy, LLC.  See the top-level NOTICE for additional details.
All rights reserved. SPDX-License-Identifier: BSD-3-Clause
*/

#include "catch.hpp"

#include "gmlc/networking/AsioContextManager.h"
#include "gmlc/networking/TcpOperations.h"
#include "gmlc/networking/addressOperations.hpp"
#include "gmlc/networking/interfaceOperations.hpp"

TEST_CASE("localHost", "[TcpClient]")
{
    asio::io_context io_context;
    auto localhost = "127.0.0.1";
    CHECK_NOTHROW(gmlc::networking::TcpConnection::create(io_context, localhost, "0"));
}

TEST_CASE("localHostString", "[TcpClient]")
{
    asio::io_context io_context;
    CHECK_NOTHROW(
        gmlc::networking::TcpConnection::create(io_context, "localhost", "0"));
}

TEST_CASE("externalAddressV4", "[TcpClient]")
{
    asio::io_context io_context;
    CHECK_NOTHROW(
        gmlc::networking::TcpConnection::create(
            io_context, gmlc::networking::getLocalExternalAddressV4(), "0"));
}

TEST_CASE("invalidString", "[TcpClient]")
{
    asio::io_context io_context;
    CHECK_THROWS(
        gmlc::networking::TcpConnection::create(io_context, "testString", "0"));
}

TEST_CASE("startReceive", "[TcpClient]")
{
    asio::io_context io_context;
    auto localhost = "127.0.0.1";
    auto connection =
        gmlc::networking::TcpConnection::create(io_context, localhost, "0");
    CHECK_NOTHROW(connection->startReceive());
    connection->closeNoWait();
}
