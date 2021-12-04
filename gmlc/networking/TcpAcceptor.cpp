/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "TcpAcceptor.h"

#include <algorithm>
#include <iostream>
#include <thread>
#include <utility>

namespace gmlc::networking {
using asio::ip::tcp;
using namespace std::chrono_literals;  // NOLINT

TcpAcceptor::TcpAcceptor(asio::io_context& io_context, tcp::endpoint& ep):
    endpoint_(ep), acceptor_(io_context)
{
    acceptor_.open(ep.protocol());
}

TcpAcceptor::TcpAcceptor(asio::io_context& io_context, uint16_t port):
    endpoint_(asio::ip::address_v4::any(), port), acceptor_(io_context, endpoint_.protocol()),
    state(AcceptingStates::CONNECTED)
{
}

bool TcpAcceptor::connect()
{
    AcceptingStates exp = AcceptingStates::OPENED;
    if (state.compare_exchange_strong(exp, AcceptingStates::CONNECTING)) {
        std::error_code ec;
        acceptor_.bind(endpoint_, ec);
        if (ec) {
            state = AcceptingStates::OPENED;
            std::cout << "acceptor error" << ec << std::endl;
            return false;
        }
        state = AcceptingStates::CONNECTED;
        return true;
    }
    return (state == AcceptingStates::CONNECTED);
}

bool TcpAcceptor::connect(std::chrono::milliseconds timeOut)
{
    if (state == AcceptingStates::HALTED) {
        state = AcceptingStates::OPENED;
    }
    AcceptingStates exp = AcceptingStates::OPENED;
    if (state.compare_exchange_strong(exp, AcceptingStates::CONNECTING)) {
        bool bindsuccess = false;
        std::chrono::milliseconds tcount{0};
        while (!bindsuccess) {
            std::error_code ec;
            acceptor_.bind(endpoint_, ec);
            if (ec) {
                if (tcount > timeOut) {
                    state = AcceptingStates::OPENED;
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                tcount += std::chrono::milliseconds(200);
            } else {
                state = AcceptingStates::CONNECTED;
                bindsuccess = true;
            }
        }
        return bindsuccess;
    }
    return (state == AcceptingStates::CONNECTED);
}

/** start the acceptor*/
bool TcpAcceptor::start(TcpConnection::pointer conn)
{
    if (!conn) {
        if (accepting.isActive()) {
            accepting.trigger();
        }
        std::cout << "tcpconnection is not valid" << std::endl;
        return false;
    }
    if (state != AcceptingStates::CONNECTED) {
        conn->close();
        if (accepting.isActive()) {
            accepting.trigger();
        }
        std::cout << "acceptor is not in a connected state" << std::endl;
        return false;
    }
    if (accepting.activate()) {
        auto& socket = conn->socket();
        acceptor_.listen();
        auto ptr = shared_from_this();
        acceptor_.async_accept(socket,
                               [this, apointer = std::move(ptr), connection = std::move(conn)](
                                   const std::error_code& error) {
                                   handle_accept(apointer, connection, error);
                               });
        return true;
    }

    std::cout << "acceptor is already active" << std::endl;
    conn->close();
    return false;
}

/** close the acceptor*/
void TcpAcceptor::close()
{
    state = AcceptingStates::HALTED;
    acceptor_.close();
    accepting.wait();
}

std::string TcpAcceptor::to_string() const
{
    auto str = endpoint_.address().to_string();
    str += ':';
    str += std::to_string(endpoint_.port());
    return str;
}
void TcpAcceptor::handle_accept(TcpAcceptor::pointer ptr,
                                TcpConnection::pointer new_connection,
                                const std::error_code& error)
{
    if (state.load() != AcceptingStates::CONNECTED) {
        asio::socket_base::linger optionLinger(true, 0);
        std::error_code ec;
        new_connection->socket().set_option(optionLinger, ec);
        new_connection->close();
        accepting.reset();
        return;
    }
    if (!error) {
        if (acceptCall) {
            accepting.reset();
            acceptCall(std::move(ptr), std::move(new_connection));
            if (!accepting.isActive()) {
                accepting.trigger();
            }
        } else {
            asio::socket_base::linger optionLinger(true, 0);
            try {
                new_connection->socket().set_option(optionLinger);
            }
            catch (...) {
            }
            new_connection->close();
            accepting.reset();
        }
    } else if (error != asio::error::operation_aborted) {
        if (errorCall) {
            errorCall(std::move(ptr), error);
        } else {
            std::cerr << " error in accept::" << error.message() << std::endl;
        }
        asio::socket_base::linger optionLinger(true, 0);
        try {
            new_connection->socket().set_option(optionLinger);
        }
        catch (...) {
        }
        new_connection->close();
        accepting.reset();
    } else {
        new_connection->close();
        accepting.reset();
    }
}

}  // namespace helics::tcp
