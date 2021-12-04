/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance
for Sustainable Energy, LLC.  See the top-level NOTICE for additional details.
All rights reserved. SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

/** @file
@details function in this file are common function used between the different
TCP comms */

#include "TcpHelperClasses.h"

#include <chrono>
#include <string>
#include <memory>

class AsioContextManager;
namespace asio {
class io_context;
}  // namespace asio

namespace gmlc::networking {
/** establish a connection to a server by as associated timeout*/
TcpConnection::pointer makeConnection(
    asio::io_context& io_context,
    const std::string& connection,
    const std::string& port,
    size_t bufferSize,
    std::chrono::milliseconds timeOut);

TcpConnection::pointer generateConnection(
    std::shared_ptr<AsioContextManager>& ioctx,
    const std::string& address);

}  // namespace gmlc::networking
