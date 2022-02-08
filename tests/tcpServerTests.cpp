/*
Copyright (c) 2017-2022,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance
for Sustainable Energy, LLC.  See the top-level NOTICE for additional details.
All rights reserved. SPDX-License-Identifier: BSD-3-Clause
*/

#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

#include "gmlc/networking/TcpOperations.h"
#include "gmlc/networking/addressOperations.hpp"
#include "gmlc/networking/interfaceOperations.hpp"
using namespace gmlc::networking;

TEST_CASE("localHost", "[TcpServer]")
{
    asio::io_context io_context;
    auto localhost = "127.0.0.1";
    TcpServer::pointer spt =
        TcpServer::create(io_context, localhost, "0", false, 10192);
    CHECK(spt->isReady());
}

TEST_CASE("externalAddress", "[TcpServer]")
{
    asio::io_context io_context;
    auto ex = getLocalExternalAddressV4();
    TcpServer::pointer spt =
        TcpServer::create(io_context, ex, "0", false, 10192);
    CHECK(spt->isReady());
}

TEST_CASE("localHostString", "[TcpServer]")
{
    asio::io_context io_context;
    TcpServer::pointer spt =
        TcpServer::create(io_context, "localhost", "0", false, 10192);
    CHECK(spt->isReady());
}
TEST_CASE("invalidString", "[TcpServer]")
{
    asio::io_context io_context;
    CHECK_THROWS(
        TcpServer::create(io_context, "testString", "0", false, 10192));
}
size_t dataFunc(TcpConnection::pointer cpt, const char* c, size_t t)
{
    std::cout << "test";
    return 10;
}

TEST_CASE("dataCall", "[TcpServer]") {
    asio::io_context io_context;
    asio::io_context io_context2;

    auto spt = TcpServer::create(io_context, "localhost", "0", false, 10192);
    auto cpt = TcpConnection::create(io_context2, "localhost", "0");
    spt->setDataCall(&dataFunc);


    
}

TEST_CASE("errorCall", "[TcpServer]") {
    asio::io_context io_context;

}