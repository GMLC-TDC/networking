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
#include "gmlc/networking/AsioContextManager.h"
using namespace gmlc::networking;

void handler(const std::error_code& e, std::size_t bytes_transferred)
{
    std::cout << "response bytes received: " << bytes_transferred << '\n';
}

size_t clientDataFunc(TcpConnection::pointer pt, const char* c, size_t t)
{
    std::cout << "CLIENT DATA: " << c << '\n';
    std::string s = "client reply\n";
    const int dataSize = s.size();
    char* data = new char[dataSize];
    strcpy(data, s.c_str());
    pt->send_async(data, dataSize, handler);
    return sizeof(c);
}

size_t serverDataFunc(TcpConnection::pointer pt, const char* c, size_t t)
{
    std::cout << "SERVER DATA: " << c << '\n';
    std::string s = "server reply\n";
    const int dataSize = s.size();
    char* data = new char[dataSize];
    strcpy(data, s.c_str());
    pt->send_async(data, dataSize, handler);
    return sizeof(c);
}

bool errorFunc(TcpConnection::pointer cpt, const std::error_code e) {
    std::cout << "ERROR: " << e << '\n';
    return true;
}

void logFunc(int loglevel, const std::string logmessage) {
    std::cout << "LOG: " << logmessage << '\n';
}

void server(std::shared_ptr<gmlc::networking::AsioContextManager> &io_context)
{
    TcpServer::pointer spt = TcpServer::create(
        io_context->getBaseContext(),
        std::string("localhost"),
        "49888",
        true);
    spt->setDataCall(serverDataFunc);
    spt->setErrorCall(errorFunc);
    spt->setLoggingFunction(logFunc);
    if (spt->isReady()) {
        spt->start();
    }
    io_context->getBaseContext().run();
}

void client(std::shared_ptr<gmlc::networking::AsioContextManager> &io_context)
{
   std::chrono::milliseconds timeOut = std::chrono::milliseconds(0);
   auto cpt = establishConnection(
        io_context->getBaseContext(),
        std::string("localhost"),
        "49888",
        timeOut);
    if (cpt != nullptr) {
        cpt->setDataCall(clientDataFunc);
        cpt->setErrorCall(errorFunc);
        cpt->setLoggingFunction(logFunc);
        cpt->startReceive();
        CHECK(cpt->isConnected());
        std::string s = "test";
        const int dataSize = s.size();
        char* data = new char[dataSize];
        strcpy(data, s.c_str());
        cpt->send_async(data, dataSize, handler);
        CHECK(cpt->isReceiving());
        io_context->startContextLoop();
    }
}

TEST_CASE("tcpOperationsTest", "[TcpOps]")
{
    auto io_context_client = gmlc::networking::AsioContextManager::getContextPointer(
        "io_context_client");
    auto io_context_server = gmlc::networking::AsioContextManager::getContextPointer(
        "io_context_server");
    std::thread s (server, io_context_server);
    s.detach();
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    std::thread c(client, io_context_client);
    c.join();
    io_context_server->getBaseContext().stop();
    io_context_server->closeContext();
}

TEST_CASE("estabilshConnectionTest0", "[TcpOps]") {
    auto io_context =
        gmlc::networking::AsioContextManager::getContextPointer(
            "io_context");
    std::chrono::milliseconds timeOut = std::chrono::milliseconds(0);
    auto cpt = establishConnection(
        io_context->getBaseContext(),
        std::string("localhost"),
        "49888",
        timeOut);
    CHECK(cpt != nullptr);
}
TEST_CASE("estabilshConnectionTestTimeOut", "[TcpOps]")
{
    auto io_context =
        gmlc::networking::AsioContextManager::getContextPointer("io_context");
    std::chrono::milliseconds timeOut = std::chrono::milliseconds(300);
    auto cpt = establishConnection(
        io_context->getBaseContext(),
        std::string("localhost"),
        "49888",
        timeOut);
    CHECK(cpt == nullptr);
}
