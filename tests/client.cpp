/*
Copyright (c) 2017-2022,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance
for Sustainable Energy, LLC.  See the top-level NOTICE for additional details.
All rights reserved. SPDX-License-Identifier: BSD-3-Clause
*/

#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"
#include <stdlib.h>
#include <thread>

#include "gmlc/networking/TcpOperations.h"
#include "gmlc/networking/addressOperations.hpp"
#include "gmlc/networking/interfaceOperations.hpp"
using namespace gmlc::networking;

size_t dataFunc(TcpConnection::pointer pt, const char* c, size_t t)
{
    std::cout << "data received: " << c << '\n';
    return 10;
}
bool errorFunc(TcpConnection::pointer cpt, const std::error_code e)
{
    std::cout << "ERROR: " << e << '\n';
    return true;
}

void logFunc(int loglevel, const std::string logmessage)
{
    std::cout << logmessage;
}

TEST_CASE("TcpConnectionCreateTest", "[tcpOps]")
{
    asio::io_context io_context;
    auto cpt =
        TcpConnection::create(io_context, std::string("localhost"), "49888");
    cpt->setDataCall(dataFunc);
    cpt->setErrorCall(errorFunc);
    cpt->setLoggingFunction(logFunc);
    cpt->startReceive();
    CHECK(cpt->isConnected());
    cpt->send("message from client");
    io_context.run();
    cpt->closeNoWait();
}

TEST_CASE("establishConnectionTest", "[tcpOps]")
{
    asio::io_context io_context;
    std::chrono::milliseconds timeOut = std::chrono::milliseconds(300);
    auto cpt = establishConnection(
        io_context, std::string("localhost"), "49888", timeOut);
    cpt->setDataCall(dataFunc);
    cpt->setErrorCall(errorFunc);
    cpt->setLoggingFunction(logFunc);
    cpt->startReceive();
    CHECK(cpt->isConnected());
    cpt->send("message from client");
    io_context.run();
    cpt->closeNoWait();
}
