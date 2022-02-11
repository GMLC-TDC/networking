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
    std::cout << "CLIENT DATA: " << c << '\n';
    return t;
}
bool errorFunc(TcpConnection::pointer cpt, const std::error_code e)
{
    std::cout << "CLIENT ERROR: " << e << '\n';
    return true;
}

void logFunc(int loglevel, const std::string& logmessage)
{
    std::cout << "CLIENT LOG: " << logmessage << '\n';
}

TEST_CASE("SendTest", "[standaloneClient]")
{
    bool withServer = false;
    std::cout << "with server(y/n)? ";
    char response = 'n';
    std::cin >> response;
    if (response == 'y' || response == 'Y') {
        withServer = true;
    }

    auto io_context_connection =
        gmlc::networking::AsioContextManager::getContextPointer(
            "io_context_connection");

    auto cpt = TcpConnection::create(
        io_context_connection->getBaseContext(),
        std::string("localhost"),
        "49888");
    std::chrono::milliseconds timeOut = std::chrono::milliseconds(5000);
    if (cpt != nullptr) {
        cpt->setDataCall(dataFunc);
        cpt->setErrorCall(errorFunc);
        cpt->setLoggingFunction(logFunc);
        cpt->startReceive();
        if (cpt->isConnected()) {
            if (withServer == true) {
                CHECK_NOTHROW(cpt->send("msg from client\n"));
            } else {
                CHECK_THROWS(cpt->send("msg from client\n"));
            }
        }
        io_context_connection->startContextLoop();
    }
}

TEST_CASE("AsyncSendTest", "[standaloneClient]")
{
    auto io_context_connection =
        gmlc::networking::AsioContextManager::getContextPointer(
            "io_context_connection");

    auto cpt = TcpConnection::create(
        io_context_connection->getBaseContext(),
        std::string("localhost"),
        "49888");
    if (cpt != nullptr) {
        cpt->setDataCall(dataFunc);
        cpt->setErrorCall(errorFunc);
        cpt->setLoggingFunction(logFunc);
        cpt->startReceive();
        CHECK(cpt->isConnected());
        std::string s = "async msg from client\n";
        const int dataSize = s.size();
        char* data = new char[dataSize];
        char* receiveData = new char[1024];
        strcpy(data, s.c_str());
        cpt->send_async(data, dataSize, handler);
        CHECK(cpt->isReceiving());
        cpt->async_receive(receiveData, 1024, handler);
        io_context_connection->startContextLoop();
    }
}
