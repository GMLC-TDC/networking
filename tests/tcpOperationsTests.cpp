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

void server()
{
    asio::io_context io_context;
    io_context.run();
    auto all = "0.0.0.0";
    // auto localhost = "127.0.0.1";
    TcpServer::pointer spt =
        TcpServer::create(io_context, all, "49700", "true", 10192);
    if (spt->isReady()) {
        spt->start();
    }
}
void client()
{
    asio::io_context io_context;
    io_context.run();
    std::chrono::milliseconds timeOut = std::chrono::milliseconds(1000);
    auto localhost = "127.0.0.1";
    TcpConnection::pointer cpt =
        establishConnection(io_context, localhost, "49700", timeOut);
}

TEST_CASE("tcpOperationsTest", "[TcpOps]")
{
    std::thread s(server);
    std::thread c(client);

    c.join();
    s.join();
}
