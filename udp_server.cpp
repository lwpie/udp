#include "udp_pack.hpp"

#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <iostream>

using boost::asio::ip::udp;

class Server {
public:
  Server(const udp::endpoint &listen_endpoint)
      : _socket(_io_service, listen_endpoint), _deadline(_io_service) {
    _deadline.expires_at(boost::posix_time::pos_infin);
    check_deadline();
  }

  std::size_t receive(const boost::asio::mutable_buffer &buffer,
                      boost::posix_time::time_duration timeout,
                      boost::system::error_code &ec) {
    _deadline.expires_from_now(timeout);

    ec = boost::asio::error::would_block;
    std::size_t length = 0;

    _socket.async_receive_from(
        buffer, _sender_endpoint,
        boost::bind(&Server::handle_receive, _1, _2, &ec, &length));

    do
      _io_service.run_one();
    while (ec == boost::asio::error::would_block);

    return length;
  }

  void store() {
    // Server::_pool.insert(_sender_endpoint);
    // for (udp::endpoint endpoint : Server::_pool)
    //   std::cout << endpoint.address().to_string() << " " << endpoint.port()
    //             << std::endl;
    std::string endpoint = _sender_endpoint.address().to_string() + ':' +
                           std::to_string(_sender_endpoint.port());
    Server::_pool.insert(endpoint);
    // for (std::string endpoint : Server::_pool)
    //   std::cout << endpoint << std::endl;
  }

  std::size_t send(boost::posix_time::time_duration timeout,
                   boost::system::error_code &ec) {
    _deadline.expires_from_now(timeout);

    ec = boost::asio::error::would_block;
    std::size_t length;

    std::stringstream ss;
    Server::_pool.serialized(ss);
    std::string buffer = ss.str();

    _socket.async_send_to(
        boost::asio::buffer(buffer.c_str(), buffer.length()), _sender_endpoint,
        boost::bind(&Server::handle_send, _1, _2, &ec, &length));

    do
      _io_service.run_one();
    while (ec == boost::asio::error::would_block);

    return length;
  }

private:
  static udp_endpoint_set _pool;

  boost::asio::io_service _io_service;
  udp::socket _socket;
  boost::asio::deadline_timer _deadline;
  udp::endpoint _sender_endpoint;

  void check_deadline() {
    if (_deadline.expires_at() <=
        boost::asio::deadline_timer::traits_type::now()) {
      _socket.cancel();
      _deadline.expires_at(boost::posix_time::pos_infin);
    }
    _deadline.async_wait(boost::bind(&Server::check_deadline, this));
  }

  static void handle_receive(const boost::system::error_code &ec,
                             std::size_t length,
                             boost::system::error_code *out_ec,
                             std::size_t *out_length) {
    *out_ec = ec;
    *out_length = length;
  }

  static void handle_send(const boost::system::error_code &ec,
                          std::size_t length, boost::system::error_code *out_ec,
                          std::size_t *out_length) {
    *out_ec = ec;
    *out_length = length;
  }
};

constexpr int max_length = 4096;
constexpr int timeout = 10;

udp_endpoint_set Server::_pool;

int main(int argc, char **argv) {
  try {
    if (argc != 3) {
      std::cerr << "Usage: udp_server <listen_addr> <listen_port>" << std::endl;
      return 1;
    }

    udp::endpoint listen_endpoint(
        boost::asio::ip::address::from_string(argv[1]), std::atoi(argv[2]));

    Server s(listen_endpoint);

    while (true) {
      char data[max_length];
      boost::system::error_code ec;
      std::size_t n = s.receive(boost::asio::buffer(data),
                                boost::posix_time::seconds(timeout), ec);

      if (ec) {
        std::cerr << "Receive error: " << ec.message() << std::endl;
      } else {
        std::cout << "Received: ";
        std::cout.write(data, n);
        std::cout << std::endl;
        s.store();
        n = s.send(boost::posix_time::seconds(timeout), ec);
        if (ec) {
          std::cerr << "Send error: " << ec.message() << std::endl;
        } else {
          std::cout << "Sent: " << n << std::endl;
        }
      }
    }
  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  }

  return 0;
}