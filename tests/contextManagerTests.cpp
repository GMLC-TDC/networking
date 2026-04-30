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

#include "gmlc/networking/AsioContextManager.h"

TEST_CASE("getContextPointerTest", "[contextManager]")
{
    ContextCleanupGuard contextCleanup("io_context");

    auto io_context =
        gmlc::networking::AsioContextManager::getContextPointer("io_context");
    auto new_context =
        gmlc::networking::AsioContextManager::getContextPointer("io_context");
    asio::io_context* cntxt_p = &io_context->getBaseContext();
    asio::io_context* new_cntxt_p = &new_context->getBaseContext();
    CHECK(cntxt_p == new_cntxt_p);
}

TEST_CASE("closeContext", "[contextManager]")
{
    ContextCleanupGuard contextCleanup("io_context");

    auto io_context =
        gmlc::networking::AsioContextManager::getContextPointer("io_context");
    io_context->startContextLoop();
    io_context->closeContext();
    CHECK_FALSE(io_context->isRunning());
}

TEST_CASE("getNameTest", "[contextManager]")
{
    ContextCleanupGuard contextCleanup("io_context");

    auto context_pointer =
        gmlc::networking::AsioContextManager::getContextPointer("io_context");
    CHECK(context_pointer->getName() == "io_context");
}
TEST_CASE("getContext", "[contextManager]")
{
    ContextCleanupGuard contextCleanup("io_context");

    auto context_pointer =
        gmlc::networking::AsioContextManager::getContextPointer("io_context");
    auto* p1 = &context_pointer->getBaseContext();
    auto* p2 = &gmlc::networking::AsioContextManager::getContext("io_context");
    CHECK(p1 == p2);
}

TEST_CASE("startContextTest", "[contextManager]")
{
    ContextCleanupGuard contextCleanup("io_context");

    auto context_pointer =
        gmlc::networking::AsioContextManager::getContextPointer("io_context");
    CHECK_NOTHROW(context_pointer->startContextLoop());
}
TEST_CASE("runContextTest", "[contextManager]")
{
    ContextCleanupGuard contextCleanup("io_context");

    auto context_pointer =
        gmlc::networking::AsioContextManager::getContextPointer(
            std::string("io_context"));
    CHECK_NOTHROW(
        gmlc::networking::AsioContextManager::runContextLoop(
            std::string("io_context")));
}
TEST_CASE("runContextTestFail", "[contextManager]")
{
    try {
        static_cast<void>(gmlc::networking::AsioContextManager::runContextLoop(
            "nonexistent"));
        FAIL("expected runContextLoop to throw for a missing context");
    }
    catch (const std::invalid_argument& e) {
        CHECK(
            std::string(e.what()) ==
            "the context name specified was not available");
    }
}
