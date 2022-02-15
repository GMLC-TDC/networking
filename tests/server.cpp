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

#include "gmlc/networking/AsioContextManager.h"
#include "gmlc/networking/TcpOperations.h"
#include "gmlc/networking/addressOperations.hpp"
#include "gmlc/networking/interfaceOperations.hpp"

using namespace gmlc::networking;

void handler(const std::error_code& e, std::size_t bytes_transferred)
{
    std::cout << "transferred: " << bytes_transferred << '\n';
}

size_t dataFunc(TcpConnection::pointer pt, const char* c, size_t t)
{
    std::cout << "SERVER DATA: " << c << '\n';
    std::string s = "server reply\n";
    const int dataSize = s.size();
    char* data = new char[dataSize];
    strcpy(data, s.c_str());
    pt->send_async(data, dataSize, handler);
    gmlc::networking::AsioContextManager::getContextPointer("io_context_server")
        ->getBaseContext()
        .stop();
    pt->closeNoWait();
    return t;
}
bool errorFunc(TcpConnection::pointer cpt, const std::error_code e)
{
    std::cout << "SERVER ERROR: " << e << '\n';
    return true;
}

void logFunc(int loglevel, const std::string& logmessage)
{
    std::cout << "SERVER LOG: " << logmessage << '\n';
}

void server(
    std::shared_ptr<gmlc::networking::AsioContextManager>& io_context_server)
{
    TcpServer::pointer spt = TcpServer::create(
        io_context_server->getBaseContext(),
        std::string("localhost"),
        "49888",
        true);
    spt->setDataCall(dataFunc);
    spt->setErrorCall(errorFunc);
    spt->setLoggingFunction(logFunc);

    if (spt->isReady()) {
        spt->start();
    }
    io_context_server->getBaseContext().run();
}

TEST_CASE("serverTest", "[standaloneServer]")
{
    std::cout << "server will wait 10 seconds before closing\n";
    auto io_context_server =
        gmlc::networking::AsioContextManager::getContextPointer(
            "io_context_server");
    std::thread s(server, std::ref(io_context_server));
    s.detach();
    std::this_thread::sleep_for(std::chrono::milliseconds(10000));
    // give enough time to set up client if needed
    std::cout << "server finishing\n";
    io_context_server->getBaseContext().stop();
    io_context_server->closeContext();
}
