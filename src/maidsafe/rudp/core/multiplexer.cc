/*******************************************************************************
 *  Copyright 2012 MaidSafe.net limited                                        *
 *                                                                             *
 *  The following source code is property of MaidSafe.net limited and is not   *
 *  meant for external use.  The use of this code is governed by the licence   *
 *  file licence.txt found in the root of this directory and also on           *
 *  www.maidsafe.net.                                                          *
 *                                                                             *
 *  You are not free to copy, amend or otherwise use this source code without  *
 *  the explicit written permission of the board of directors of MaidSafe.net. *
 ******************************************************************************/
// Original author: Christopher M. Kohlhoff (chris at kohlhoff dot com)


#include <cassert>

#include "maidsafe/rudp/core/multiplexer.h"
#include "maidsafe/rudp/packets/packet.h"
#include "maidsafe/rudp/utils.h"
#include "maidsafe/rudp/log.h"

namespace asio = boost::asio;
namespace ip = boost::asio::ip;
namespace bs = boost::system;

namespace maidsafe {

namespace rudp {

namespace detail {

Multiplexer::Multiplexer(asio::io_service &asio_service) //NOLINT
  : socket_(asio_service),
    receive_buffer_(Parameters::max_size),
    sender_endpoint_(),
    dispatcher_() {}

Multiplexer::~Multiplexer() {}

ReturnCode Multiplexer::Open(const ip::udp::endpoint &endpoint) {
  if (socket_.is_open()) {
    DLOG(WARNING) << "Multiplexer already open.";
    return kAlreadyStarted;
  }

  bs::error_code ec;
  socket_.open(endpoint.protocol(), ec);

  if (ec) {
    DLOG(ERROR) << "Multiplexer socket opening error: " << ec.message();
    return kInvalidAddress;
  }

  ip::udp::socket::non_blocking_io nbio(true);
  socket_.io_control(nbio, ec);

  if (ec) {
    DLOG(ERROR) << "Multiplexer setting option error: " << ec.message();
    return kSetOptionFailure;
  }

  if (IsValid(endpoint)) {
    socket_.bind(endpoint, ec);
    if (ec) {
      DLOG(ERROR) << "Multiplexer socket binding error: " << ec.message();
      return kBindError;
    }
  }

  return kSuccess;
}

bool Multiplexer::IsOpen() const {
  return socket_.is_open();
}

void Multiplexer::Close() {
  bs::error_code ec;
  socket_.close(ec);
  if (ec)
    DLOG(WARNING) << "Multiplexer closing error: " << ec.message();
}

void Multiplexer::PrintSendError(const boost::asio::ip::udp::endpoint &endpoint,
                                 boost::system::error_code ec) const {
  DLOG(ERROR) << "Error sending to << " << endpoint << " - " << ec.message();
}

}  // namespace detail

}  // namespace rudp

}  // namespace maidsafe
