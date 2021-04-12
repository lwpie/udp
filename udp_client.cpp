#include "udp_pack.hpp"

#include <boost/asio.hpp>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>

using boost::asio::ip::udp;

enum { max_length = 4096 };

int main(int argc, char *argv[]) {
  udp_endpoint_set pool;
  try {
    if (argc != 3) {
      std::cerr << "Usage: udp_client <host> <port>" << std::endl;
      return 1;
    }

    boost::asio::io_context io_context;

    udp::socket s(io_context, udp::endpoint(udp::v4(), 0));

    udp::resolver resolver(io_context);
    udp::resolver::results_type endpoints =
        resolver.resolve(udp::v4(), argv[1], argv[2]);

    std::cout << "Enter message: ";
    char request[max_length];
    std::cin.getline(request, max_length);
    size_t request_length = std::strlen(request);
    s.send_to(boost::asio::buffer(request, request_length), *endpoints.begin());

    char reply[max_length];
    udp::endpoint sender_endpoint;
    size_t reply_length =
        s.receive_from(boost::asio::buffer(reply, max_length), sender_endpoint);
    std::cout << "Reply length: " << reply_length << std::endl;
    std::stringstream ss;
    ss.write(reply, reply_length);
    try {
      pool.deserialized(ss);
      for (std::string endpoint : pool)
        std::cout << endpoint << std::endl;
    } catch (std::exception &e) {
      std::cout << e.what() << std::endl;
    }
  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}