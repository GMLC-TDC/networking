/*
Copyright (c) 2017-2022,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance
for Sustainable Energy, LLC.  See the top-level NOTICE for additional details.
All rights reserved. SPDX-License-Identifier: BSD-3-Clause
*/

#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

#include "gmlc/networking/AsioContextManager.h"
#include "gmlc/networking/TcpOperations.h"
#include "gmlc/networking/addressOperations.hpp"
#include "gmlc/networking/interfaceOperations.hpp"

using namespace gmlc::networking;

TEST_CASE("localHost", "[TcpClient]")
{
    asio::io_context io_context;
    auto localhost = "127.0.0.1";
    CHECK_NOTHROW(TcpConnection::create(io_context, localhost, "0"));
}

TEST_CASE("localHostString", "[TcpClient]")
{
    asio::io_context io_context;
    CHECK_NOTHROW(TcpConnection::create(io_context, "localhost", "0"));
}

TEST_CASE("externalAddressV4", "[TcpClient]")
{
    asio::io_context io_context;
    auto ex = getLocalExternalAddressV4();
    CHECK_NOTHROW(TcpConnection::create(io_context, ex, "0"));
}

TEST_CASE("invalidString", "[TcpClient]")
{
    asio::io_context io_context;
    CHECK_THROWS(TcpConnection::create(io_context, "testString", "0"));
}

TEST_CASE("startReceive", "[TcpClient]")
{
    asio::io_context io_context;
    auto localhost = "127.0.0.1";
    auto connection = TcpConnection::create(io_context, localhost, "0");
    CHECK_NOTHROW(connection->startReceive());
}

void handler(const std::error_code& e, std::size_t bytes_transferred)
{
    std::cout << "client transferred: " << bytes_transferred << '\n';
}

size_t dataFunc(TcpConnection::pointer pt, const char* c, size_t t)
{
    std::cout << "CLIENT DATA: " << c << '\n';
    std::string s = "connection reply\n";
    const int dataSize = s.size();
    char* data = new char[dataSize];
    strcpy(data, s.c_str());
    pt->send_async(data, dataSize, handler);
    pt->close();
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

TEST_CASE("SendTest", "[tcpClient]")
{
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
            std::string s = "msg from client\n";
            const int dataSize = s.size();
            char* data = new char[dataSize];
            strcpy(data, s.c_str());
            CHECK_THROWS(cpt->send(data, dataSize));
            // without a running server, above should throw an exception
        }
        io_context_connection->startContextLoop();
    }
}

TEST_CASE("AsyncSendTest", "[tcpClient]")
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
        CHECK_NOTHROW(cpt->async_receive(receiveData, 1024, handler));
        io_context_connection->startContextLoop();
    }
}
