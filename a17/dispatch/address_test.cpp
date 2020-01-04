#include "catch.hpp"

#include <stdlib.h>

#include "address.h"

namespace a17 {
namespace dispatch {
namespace test {

TEST_CASE("Address", "[address]") {
  a17::dispatch::Address addr;
  std::shared_ptr<spdlog::logger> logger = spdlog::get("Address");

  SECTION("Constructors") {
    a17::dispatch::Address address;
    REQUIRE(address.ip() == a17::dispatch::OwnAddress::instance().address());
    REQUIRE(address.port() == 0);
    REQUIRE(address.address() == "tcp://" + a17::dispatch::OwnAddress::instance().address() + ":0");

    a17::dispatch::Address address2(1024);
    REQUIRE(address2.address() ==
            "tcp://" + a17::dispatch::OwnAddress::instance().address() + ":1024");
  }

  SECTION("To string") {
    a17::dispatch::Address address("inproc", "process", 45);
    REQUIRE(address.address() == "inproc://process:45");
    REQUIRE(static_cast<std::string>(address) == "inproc://process:45");
  }

  // DISABLED because it breaks in a docker container.
  // TODO(kgreenek): Figure out what this was supposed to be testing and re-enable / delete.
  //SECTION("Interface selection") {
  //  struct ifaddrs *ifaddr, *ifa;
  //  if (getifaddrs(&ifaddr) < 0) {
  //    if (logger) logger->warn("Could not obtain iface addresses, skipping test");
  //    return;
  //  }
  //  std::string valid_interface;
  //  for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
  //    if (!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_INET) continue;
  //    if (strcmp(ifa->ifa_name, "en0") == 0) {
  //      valid_interface = "en0";
  //    } else if (strcmp(ifa->ifa_name, "wlan0") == 0) {
  //      valid_interface = "wlan0";
  //    } else if (strcmp(ifa->ifa_name, "wlp2s0") == 0) {
  //      valid_interface = "wlp2s0";
  //    }
  //  }
  //  if (valid_interface.empty()) {
  //    if (logger) logger->warn("Could not find en0 or wlan0 interfaces, skipping test");
  //  }
  //  setenv("A17_DISPATCH_INTERFACE", valid_interface.c_str(), 1);
  //  a17::dispatch::OwnAddress own_address;
  //  REQUIRE(own_address.address() != "127.0.0.1");
  //  REQUIRE(own_address.interface() == valid_interface);
  //}

  SECTION("Invalid interface") {
    setenv("A17_DISPATCH_INTERFACE", "blah", 1);
    a17::dispatch::OwnAddress own_address;
    REQUIRE(own_address.address() == "127.0.0.1");
    REQUIRE(own_address.interface().empty());
  }

  SECTION("Parse from string") {
    a17::dispatch::Address address = a17::dispatch::Address::parse("http://127.0.0.1:4004");
    REQUIRE(address.protocol() == "http");
    REQUIRE(address.ip() == "127.0.0.1");
    REQUIRE(address.port() == 4004);

    a17::dispatch::Address address2 = a17::dispatch::Address::parse("127.0.0.1");
    REQUIRE(address2.protocol() == "tcp");
    REQUIRE(address2.ip() == "127.0.0.1");
    REQUIRE(address2.port() == 0);

    a17::dispatch::Address address3 = a17::dispatch::Address::parse("127.0.0.1:4004");
    REQUIRE(address3.protocol() == "tcp");
    REQUIRE(address3.ip() == "127.0.0.1");
    REQUIRE(address3.port() == 4004);

    a17::dispatch::Address address4 = a17::dispatch::Address::parse("ojasdfoioi:sdf");
    REQUIRE(address4.protocol() == "tcp");
    REQUIRE(address4.ip() == "ojasdfoioi");
    REQUIRE(address4.port() == 0);
  }
}

}  // namespace test
}  // namespace dispatch
}  // namespace a17
