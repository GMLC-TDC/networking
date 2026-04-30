/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance
for Sustainable Energy, LLC.  See the top-level NOTICE for additional details.
All rights reserved. SPDX-License-Identifier: BSD-3-Clause
*/

#include "catch.hpp"

#include <string>

#include "gmlc/networking/addressOperations.hpp"

TEST_CASE("stripProtocol", "[address_operations]")
{
    CHECK(gmlc::networking::stripProtocol("tcp://127.0.0.1") == "127.0.0.1");
}

TEST_CASE("removeProtocol", "[address_operations]")
{
    std::string networkAddress = "tcp://127.0.0.1";
    gmlc::networking::removeProtocol(networkAddress);
    CHECK(networkAddress == "127.0.0.1");
}

TEST_CASE("addProtocol", "[address_operations]")
{
    CHECK(
        gmlc::networking::addProtocol(
            "127.0.0.1", gmlc::networking::InterfaceTypes::TCP) ==
        "tcp://127.0.0.1");
}

TEST_CASE("insertProtocol", "[address_operations]")
{
    std::string networkAddress = "127.0.0.1";
    gmlc::networking::insertProtocol(
        networkAddress, gmlc::networking::InterfaceTypes::TCP);
    CHECK(networkAddress == "tcp://127.0.0.1");
}

TEST_CASE("add_check_detection", "[address_operations]")
{
    CHECK(gmlc::networking::isIpv6("FEDC:BA98:7654:3210:FEDC:BA98:7654:3210"));
    CHECK(gmlc::networking::isIpv6("::192.9.5.5"));
    CHECK(gmlc::networking::isIpv6("http://[1080::8:800:200C:417A]/foo"));
    CHECK(gmlc::networking::isIpv6("::0"));
    CHECK_FALSE(gmlc::networking::isIpv6("192.9.5.5"));
    CHECK_FALSE(gmlc::networking::isIpv6("tcp://192.9.5.5:80"));
}
