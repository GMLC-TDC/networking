/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance
for Sustainable Energy, LLC.  See the top-level NOTICE for additional details.
All rights reserved. SPDX-License-Identifier: BSD-3-Clause
*/

#include "TcpConnection.h"

#include <algorithm>
#include <iostream>
#include <thread>
#include <utility>

namespace gmlc::networking {
using asio::ip::tcp;
using namespace std::chrono_literals;  // NOLINT

std::atomic<int> TcpConnection::idcounter{10};

void TcpConnection::startReceive()
{
    if (triggerhalt) {
        receivingHalt.trigger();
        return;
    }
    if (state == ConnectionStates::PRESTART) {
        receivingHalt.activate();
        connected.activate();
        state = ConnectionStates::WAITING;
    }
    ConnectionStates exp = ConnectionStates::WAITING;
    if (state.compare_exchange_strong(exp, ConnectionStates::OPERATING)) {
        if (!receivingHalt.isActive()) {
            receivingHalt.activate();
        }
        if (!triggerhalt) {
            socket_.async_read_some(
                asio::buffer(
                    data.data() + residBufferSize,
                    data.size() - residBufferSize),
                [ptr = shared_from_this()](
                    const std::error_code& err, size_t bytes) {
                    ptr->handle_read(err, bytes);
                });
            if (triggerhalt) {
                // cancel previous operation if triggerhalt is now active
                socket_.lowest_layer().cancel();
                // receivingHalt.trigger();
            }
        } else {
            state = ConnectionStates::HALTED;
            receivingHalt.trigger();
        }
    } else if (exp != ConnectionStates::OPERATING) {
        /*either halted or closed*/
        receivingHalt.trigger();
    }
}

void TcpConnection::setDataCall(
    std::function<size_t(TcpConnection::pointer, const char*, size_t)> dataFunc)
{
    if (state.load() == ConnectionStates::PRESTART) {
        dataCall = std::move(dataFunc);
    } else {
        throw(std::runtime_error(
            "cannot set data callback after socket is started"));
    }
}
void TcpConnection::setErrorCall(
    std::function<bool(TcpConnection::pointer, const std::error_code&)>
        errorFunc)
{
    if (state.load() == ConnectionStates::PRESTART) {
        errorCall = std::move(errorFunc);
    } else {
        throw(std::runtime_error(
            "cannot set error callback after socket is started"));
    }
}

void TcpConnection::setLoggingFunction(
    std::function<void(int loglevel, const std::string& logMessage)> logFunc)
{
    if (state.load() == ConnectionStates::PRESTART) {
        logFunction = std::move(logFunc);
    } else {
        throw(std::runtime_error(
            "cannot set logging function after socket is started"));
    }
}

void TcpConnection::handle_read(
    const std::error_code& error,
    size_t bytes_transferred)
{
    if (triggerhalt.load(std::memory_order_acquire)) {
        state = ConnectionStates::HALTED;
        receivingHalt.trigger();
        return;
    }
    if (!error) {
        auto used = dataCall(
            shared_from_this(),
            data.data(),
            bytes_transferred + residBufferSize);
        if (used < (bytes_transferred + residBufferSize)) {
            if (used > 0) {
                std::copy(
                    data.data() + used,
                    data.data() + bytes_transferred + residBufferSize,
                    data.data());
            }
            residBufferSize = bytes_transferred + residBufferSize - used;
        } else {
            residBufferSize = 0;
            data.assign(data.size(), 0);
        }
        state = ConnectionStates::WAITING;
        startReceive();
    } else if (error == asio::error::operation_aborted) {
        state = ConnectionStates::HALTED;
        receivingHalt.trigger();
        return;
    } else {
        // there was an error
        if (bytes_transferred > 0) {
            auto used = dataCall(
                shared_from_this(),
                data.data(),
                bytes_transferred + residBufferSize);
            if (used < (bytes_transferred + residBufferSize)) {
                if (used > 0) {
                    std::copy(
                        data.data() + used,
                        data.data() + bytes_transferred + residBufferSize,
                        data.data());
                }
                residBufferSize = bytes_transferred + residBufferSize - used;
            } else {
                residBufferSize = 0;
            }
        }
        if (errorCall) {
            if (errorCall(shared_from_this(), error)) {
                state = ConnectionStates::WAITING;
                startReceive();
            } else {
                state = ConnectionStates::HALTED;
                receivingHalt.trigger();
            }
        } else if (error != asio::error::eof) {
            if (error != asio::error::connection_reset) {
                std::cerr << "receive error " << error.message() << std::endl;
            }
            state = ConnectionStates::HALTED;
            receivingHalt.trigger();
        } else {
            state = ConnectionStates::HALTED;
            receivingHalt.trigger();
        }
    }
}

// asio::socket_base::linger optionLinger(true, 2);
// socket_.lowest_layer().set_option(optionLinger, ec);
void TcpConnection::close()
{
    closeNoWait();
    waitOnClose();
}

void TcpConnection::closeNoWait()
{
    triggerhalt.store(true);
    switch (state.load()) {
        case ConnectionStates::PRESTART:
            if (receivingHalt.isActive()) {
                receivingHalt.trigger();
            }
            break;
        case ConnectionStates::HALTED:
        case ConnectionStates::CLOSED:
            receivingHalt.trigger();
            break;
        default:
            break;
    }

    std::error_code ec;
    if (socket_.lowest_layer().is_open()) {
        socket_.lowest_layer().shutdown(tcp::socket::shutdown_both, ec);
        if (ec) {
            if ((ec.value() != asio::error::not_connected) &&
                (ec.value() != asio::error::connection_reset)) {
                std::cerr << "error occurred sending shutdown::" << ec.message()
                          << " " << ec.value() << std::endl;
            }
            ec.clear();
        }
        socket_.lowest_layer().close(ec);
    } else {
        socket_.lowest_layer().close(ec);
    }
}

/** wait on the closing actions*/
void TcpConnection::waitOnClose()
{
    if (triggerhalt.load(std::memory_order_acquire)) {
        if (connecting) {
            connected.waitActivation();
        }

        while (!receivingHalt.wait_for(std::chrono::milliseconds(200))) {
            std::cout << "wait timeout " << static_cast<int>(state.load())
                      << " " << socket_.lowest_layer().is_open() << " "
                      << receivingHalt.isTriggered() << std::endl;

            std::cout << "wait info " << context_.stopped() << " " << connecting
                      << std::endl;
        }
    } else {
        close();
    }
    state.store(ConnectionStates::CLOSED);
}

TcpConnection::pointer TcpConnection::create(
    asio::io_context& io_context,
    const std::string& connection,
    const std::string& port,
    size_t bufferSize)
{
    return pointer(new TcpConnection(io_context, connection, port, bufferSize));
}

TcpConnection::TcpConnection(
    asio::io_context& io_context,
    const std::string& connection,
    const std::string& port,
    size_t bufferSize) :
    socket_(io_context),
    context_(io_context), data(bufferSize), connecting(true),
    idcode(idcounter++)
{
    tcp::resolver resolver(io_context);
    tcp::resolver::query query(tcp::v4(), connection, port);
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
    socket_.lowest_layer().async_connect(
        *endpoint_iterator,
        [this](const std::error_code& error) { connect_handler(error); });
}

void TcpConnection::connect_handler(const std::error_code& error)
{
    if (!error) {
        connected.activate();
        socket_.lowest_layer().set_option(asio::ip::tcp::no_delay(true));
    } else {
        std::cerr << "connection error " << error.message()
                  << ": code =" << error.value() << '\n';
        connectionError = true;
        connected.activate();
    }
}
size_t TcpConnection::send(const void* buffer, size_t dataLength)
{
    if (!isConnected()) {
        if (!waitUntilConnected(300ms)) {
            std::cerr << "connection timeout waiting again" << std::endl;
        }
        if (!waitUntilConnected(200ms)) {
            std::cerr << "connection timeout twice, now returning" << std::endl;
            return 0;
        }
    }

    size_t sz{0};
    size_t sent_size{dataLength};
    size_t p{0};
    int count{0};
    while (count++ < 5 &&
           (sz = socket_.write_some(asio::buffer(
                reinterpret_cast<const char*>(buffer) + p, sent_size))) !=
               sent_size) {
        sent_size -= sz;
        p += sz;
        //   std::cerr << "DEBUG partial buffer sent" << std::endl;
    }
    if (count >= 5) {
        std::cerr << "TcpConnection send terminated " << std::endl;
        return 0;
    }
    return dataLength;

    //  assert(sz == dataLength);
    //  return sz;
}

size_t TcpConnection::send(const std::string& dataString)
{
    size_t sz;
    sz = send(&dataString[0], dataString.size());
    return sz;
    /*
            if (!isConnected()) {
                if (!waitUntilConnected(300ms)) {
                    std::cerr << "connection timeout waiting again" <<
       std::endl;
                }
                if (!waitUntilConnected(200ms)) {
                    std::cerr << "connection timeout twice, now returning" <<
       std::endl; return 0;
                }
            }
            auto sz = socket_.write_some(asio::buffer(dataString));
            assert(sz == dataString.size());
            return sz;
    */
}

size_t TcpConnection::receive(void* buffer, size_t maxDataSize)
{
    return socket_.read_some(asio::buffer(buffer, maxDataSize));
}

bool TcpConnection::waitUntilConnected(std::chrono::milliseconds timeOut)
{
    if (isConnected()) {
        return true;
    }
    if (timeOut < 0ms) {
        connected.waitActivation();
        return isConnected();
    }
    connected.wait_forActivation(timeOut);
    return isConnected();
}

}  // namespace gmlc::networking
