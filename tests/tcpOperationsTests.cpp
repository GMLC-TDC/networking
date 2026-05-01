/*
Copyright (c) 2017-2026,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance
for Sustainable Energy, LLC.  See the top-level NOTICE for additional details.
All rights reserved. SPDX-License-Identifier: BSD-3-Clause
*/

#include "catch.hpp"
#include "testCleanup.hpp"

#include <stdlib.h>
#include <string>
#include <thread>

#include "gmlc/networking/AsioContextManager.h"
#include "gmlc/networking/TcpOperations.h"
#include "gmlc/networking/addressOperations.hpp"
#include "gmlc/networking/interfaceOperations.hpp"

void handler(const std::error_code& /*e*/, std::size_t bytes_transferred)
{
    CHECK(bytes_transferred == 5);
}

void client(gmlc::networking::TcpConnection::pointer cpt)
{
    static constexpr char data[] = "test0";
    constexpr std::size_t dataSize = sizeof(data) - 1;
    cpt->send_async(data, dataSize, handler);
}

static bool isExpectedSocketShutdownError(const std::error_code& error)
{
    return (error == asio::error::eof) ||
        (error == asio::error::connection_reset) ||
        (error == asio::error::operation_aborted) ||
        (error.value() == asio::error::eof) ||
        (error.value() == asio::error::connection_reset) ||
        (error.value() == asio::error::operation_aborted);
}

TEST_CASE("asynchronousTcpOperationsTest", "[TcpOps]")
{
    ContextCleanupGuard contextCleanup("io_context_server");

    auto io_context_server =
        gmlc::networking::AsioContextManager::getContextPointer(
            "io_context_server");

    auto server_context_loop = io_context_server->startContextLoop();
    auto spt = gmlc::networking::TcpServer::create(
        io_context_server->getBaseContext(), "localhost", 19888, true);
    int itCount{0};
    while (!spt->isReady()) {
        ++itCount;
        if (itCount > 10) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }
    REQUIRE(spt->isReady());

    size_t data_recv_size;
    spt->setDataCall([&](const gmlc::networking::TcpConnection::pointer&,
                         const char* data,
                         size_t datasize) {
        CHECK(datasize == 5);
        data_recv_size = datasize;
        CHECK(std::string(data, datasize) == "test0");
        return datasize;
    });
    spt->setErrorCall([](const gmlc::networking::TcpConnection::pointer&,
                         const std::error_code& error) {
        if (isExpectedSocketShutdownError(error)) {
            return false;
        }
        INFO("Error (" << error.value() << "): " << error.message());
        CHECK(false);
        return false;
    });

    spt->setLoggingFunction([](int loglevel, const std::string& logMessage) {
        INFO(logMessage << " loglevel=" << loglevel);
    });

    spt->start();

    std::chrono::milliseconds timeOut = std::chrono::milliseconds(0);
    auto cpt = gmlc::networking::establishConnection(
        io_context_server->getBaseContext(),
        std::string("localhost"),
        "19888",
        timeOut);
    itCount = 0;
    while (!cpt->isConnected()) {
        ++itCount;
        if (itCount > 10) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    REQUIRE(cpt->isConnected());
    std::thread c(client, std::ref(cpt));
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    c.join();

    spt->close();
    cpt->close();
    cpt.reset();
    spt.reset();
    server_context_loop.reset();
    io_context_server.reset();

    CHECK(data_recv_size == 5);
}

TEST_CASE("TcpOperationsTest", "[TcpOps]")
{
    ContextCleanupGuard contextCleanup("io_context_server");
    contextCleanup.add("io_context_client");

    auto io_context_server =
        gmlc::networking::AsioContextManager::getContextPointer(
            "io_context_server");

    auto server_context_loop = io_context_server->startContextLoop();
    auto spt = gmlc::networking::TcpServer::create(
        io_context_server->getBaseContext(), "*", 19888, true);
    int itCount{0};
    while (!spt->isReady()) {
        ++itCount;
        if (itCount > 10) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }
    REQUIRE(spt->isReady());

    size_t data_recv_size;
    spt->setDataCall([&](const gmlc::networking::TcpConnection::pointer&,
                         const char* data,
                         size_t datasize) {
        CHECK(datasize == 5);
        data_recv_size = datasize;
        CHECK(std::string(data, datasize) == "test0");
        return datasize;
    });
    spt->setErrorCall([](const gmlc::networking::TcpConnection::pointer&,
                         const std::error_code& error) {
        if (isExpectedSocketShutdownError(error)) {
            return false;
        }
        INFO("Error (" << error.value() << "): " << error.message());
        CHECK(false);
        return false;
    });

    spt->setLoggingFunction([](int loglevel, const std::string& logMessage) {
        INFO(logMessage << " loglevel=" << loglevel);
    });

    spt->start();

    auto io_context_client =
        gmlc::networking::AsioContextManager::getContextPointer(
            "io_context_client");
    std::chrono::milliseconds timeOut = std::chrono::milliseconds(0);
    auto cpt = gmlc::networking::establishConnection(
        io_context_client->getBaseContext(),
        std::string("localhost"),
        "19888",
        timeOut);
    auto client_ctxt_loop = io_context_client->startContextLoop();
    itCount = 0;
    while (!cpt->isConnected()) {
        ++itCount;
        if (itCount > 10) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    REQUIRE(cpt->isConnected());
    cpt->send("test0");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    spt->close();
    cpt->close();
    cpt.reset();
    spt.reset();
    client_ctxt_loop.reset();
    server_context_loop.reset();
    io_context_client.reset();
    io_context_server.reset();

    CHECK(data_recv_size == 5);
}
