/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance
for Sustainable Energy, LLC.  See the top-level NOTICE for additional details.
All rights reserved. SPDX-License-Identifier: BSD-3-Clause
*/
#include "gmlc/networking/addressOperations.hpp"

#include "gtest/gtest.h"

using namespace gmlc::networking;

TEST(address_operations, stripProtocol)
{
    EXPECT_EQ(stripProtocol("tcp://127.0.0.1"), "127.0.0.1");
}

TEST(address_operations, removeProtocol)
{
    std::string networkAddress = "tcp://127.0.0.1";
    removeProtocol(networkAddress);
    EXPECT_EQ(networkAddress, "127.0.0.1");
}

TEST(address_operations, addProtocol)
{
    EXPECT_EQ(addProtocol("127.0.0.1", InterfaceTypes::TCP), "tcp://127.0.0.1");
}

TEST(address_operations, insertProtocol)
{
    std::string networkAddress = "127.0.0.1";
    insertProtocol(networkAddress, InterfaceTypes::TCP);
    EXPECT_EQ(networkAddress, "tcp://127.0.0.1");
}

TEST(address_operations, add_check_detection)
{
    EXPECT_TRUE(isipv6("FEDC:BA98:7654:3210:FEDC:BA98:7654:3210"));
    EXPECT_TRUE(isipv6("::192.9.5.5"));
    EXPECT_TRUE(isipv6("http://[1080::8:800:200C:417A]/foo"));
    EXPECT_TRUE(isipv6("::0"));
    EXPECT_TRUE(!isipv6("192.9.5.5"));
    EXPECT_TRUE(!isipv6("tcp://192.9.5.5:80"));
}
