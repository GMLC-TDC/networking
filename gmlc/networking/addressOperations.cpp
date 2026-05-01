/*
Copyright (c) 2017-2026,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance
for Sustainable Energy, LLC.  See the top-level NOTICE for additional details.
All rights reserved. SPDX-License-Identifier: BSD-3-Clause
*/
#include "addressOperations.hpp"

#include <algorithm>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

namespace gmlc::networking {

std::pair<std::string, std::optional<std::string>>
    extractInterfaceAndPortString(std::string_view address)
{
    auto lastColon = address.find_last_of(':');
    if (lastColon == std::string::npos || (address.size() <= lastColon + 1) ||
        (address[lastColon + 1] == '/')) {
        return {std::string(address), std::nullopt};
    }
    return {
        std::string(address.substr(0, lastColon)),
        std::string(address.substr(lastColon + 1))};
}

std::string makePortAddress(std::string_view networkInterface, int portNumber)
{
    std::string newAddress{networkInterface};
    if (portNumber != 0) {
        newAddress.push_back(':');
        newAddress.append(std::to_string(portNumber));
    }
    return newAddress;
}

std::pair<std::string, std::optional<int>>
    extractInterfaceAndPort(std::string_view address)
{
    std::pair<std::string, std::optional<int>> ret;
    auto lastColon = address.find_last_of(':');
    if (lastColon == std::string::npos) {
        ret = std::make_pair(std::string(address), std::nullopt);
    } else {
        try {
            if ((address.size() > lastColon + 1) &&
                (address[lastColon + 1] != '/')) {
                auto val =
                    std::stoi(std::string(address.substr(lastColon + 1)));
                ret.first = std::string(address.substr(0, lastColon));
                ret.second = val;
            } else {
                ret = std::make_pair(std::string(address), std::nullopt);
            }
        }
        catch (const std::invalid_argument&) {
            ret = std::make_pair(std::string(address), std::nullopt);
        }
    }

    return ret;
}

std::string stripProtocol(std::string_view networkAddress)
{
    auto loc = networkAddress.find("://");
    if (loc != std::string::npos) {
        return std::string(networkAddress.substr(loc + 3));
    }
    return std::string(networkAddress);
}

void removeProtocol(std::string& networkAddress)
{
    auto loc = networkAddress.find("://");
    if (loc != std::string::npos) {
        networkAddress.erase(0, loc + 3);
    }
}

bool isIpv6(std::string_view address)
{
    auto cntcolon = std::count(address.begin(), address.end(), ':');
    if ((cntcolon < 2) || (cntcolon > 9)) {
        // not enough colons need at least 2 and at most 9
        return false;
    }
    auto brkcnt = address.find_first_of('[');
    if (brkcnt != std::string::npos) {
        auto brkend = address.find_first_of(']', brkcnt + 2);
        if (brkend == std::string::npos) {
            // invalid format
            return false;
        }
        address = address.substr(brkcnt, brkend - brkcnt + 1);
    }
    auto prevColon = address.find_first_of(':');
    auto nextColon = address.find_first_of(':', prevColon + 1);

    int dcolon = 0;
    while (nextColon != std::string::npos) {
        if (nextColon - prevColon == 1) {
            ++dcolon;
            if (dcolon > 1) {
                return false;
            }
        }
        if (nextColon - prevColon > 5) {
            return false;
        }
        prevColon = nextColon;
        nextColon = address.find_first_of(':', nextColon + 1);
    }
    return true;
}

std::string
    addProtocol(std::string_view networkAddress, InterfaceTypes interfaceT)
{
    if (networkAddress.find("://") == std::string::npos) {
        switch (interfaceT) {
            case InterfaceTypes::IP:
            case InterfaceTypes::TCP:
                return std::string("tcp://") + std::string(networkAddress);
            case InterfaceTypes::IPC:
                return std::string("ipc://") + std::string(networkAddress);
            case InterfaceTypes::UDP:
                return std::string("udp://") + std::string(networkAddress);
            case InterfaceTypes::INPROC:
                return std::string("inproc://") + std::string(networkAddress);
        }
    }
    return std::string(networkAddress);
}

void insertProtocol(std::string& networkAddress, InterfaceTypes interfaceT)
{
    if (networkAddress.find("://") == std::string::npos) {
        switch (interfaceT) {
            case InterfaceTypes::IP:
            case InterfaceTypes::TCP:
                networkAddress.insert(0, "tcp://");
                break;
            case InterfaceTypes::IPC:
                networkAddress.insert(0, "ipc://");
                break;
            case InterfaceTypes::UDP:
                networkAddress.insert(0, "udp://");
                break;
            case InterfaceTypes::INPROC:
                networkAddress.insert(0, "inproc://");
                break;
        }
    }
}

}  // namespace gmlc::networking
