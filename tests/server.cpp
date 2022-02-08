/*
Copyright (c) 2017-2022,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance
for Sustainable Energy, LLC.  See the top-level NOTICE for additional details.
All rights reserved. SPDX-License-Identifier: BSD-3-Clause
*/

#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"
#include <thread>
#include <stdlib.h>

#include "gmlc/networking/addressOperations.hpp"
#include "gmlc/networking/interfaceOperations.hpp"
#include "gmlc/networking/TcpOperations.h"
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


void server()
{
    asio::io_context io_context;
    auto all = "tcp://0.0.0.0";
    std::string localhost = "127.0.0.1";
    TcpServer::pointer spt = TcpServer::create(
        io_context, std::string("localhost"), "49888", true, 10192);
    spt->setDataCall(dataFunc);
    spt->setErrorCall(errorFunc);
    spt->setLoggingFunction(logFunc);

    if (spt->isReady()) {
        spt->start();
    }
    io_context.run();
    spt->close();
}


TEST_CASE("serverTest", "[tcpOps]")
{
    server();
}