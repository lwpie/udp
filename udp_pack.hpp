#pragma once

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/serialization/unordered_set.hpp>
#include <sstream>
#include <string>
#include <unordered_set>

using boost::asio::ip::udp;

template <class T> class Serialization : public T {
public:
  void serialized(std::stringstream &ostream) {
    boost::archive::binary_oarchive oa(ostream);
    oa << *this;
  }

  void deserialized(std::stringstream &istream) {
    boost::archive::binary_iarchive ia(istream);
    ia >> *this;
  }

private:
  friend class boost::serialization::access;

  template <class Archive>
  void serialize(Archive &ar, const unsigned int version) {
    ar &boost::serialization::base_object<T>(*this);
  }
};

// struct udp_endpoint_hash {
//   std::size_t operator()(const udp::endpoint &endpoint) const {
//     return std::hash<std::string>()(endpoint.address().to_string() +
//                                     std::to_string(endpoint.port()));
//   }
// };

// class udp_endpoint_set
//     : public Serialization<
//           std::unordered_set<udp::endpoint, udp_endpoint_hash>> {};

class udp_endpoint_set : public Serialization<std::unordered_set<std::string>> {
};