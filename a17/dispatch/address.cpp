#include "address.h"

#if SPDLOG_VERSION >= 10000
  #include <spdlog/sinks/stdout_color_sinks.h>
#endif

namespace a17 {
namespace dispatch {

int getAddressFromSocket(std::string &address, sockaddr *addr) {
  if (!addr) return -1;
  char address_cstr[NI_MAXHOST];
  auto s = getnameinfo(addr, sizeof(struct sockaddr_in), address_cstr, NI_MAXHOST, nullptr, 0,
                       NI_NUMERICHOST);
  if (s >= 0) {
    address = std::string(address_cstr);
  }
  return s;
}

OwnAddress::OwnAddress()
    : address_("127.0.0.1"),
      broadcast_("127.0.0.1"),
      netmask_("255.255.255.255"),
      network_(inet_addr("127.0.0.1")),
      mask_(inet_addr("255.255.255.255")) {
  std::shared_ptr<spdlog::logger> logger = spdlog::get("Address");
  if (!logger) {
    try {
      logger = spdlog::stdout_color_mt("Address");
    } catch (...) {
      logger = spdlog::get("Address");
      if (!logger) {
        throw std::runtime_error("Address logger get() failed twice.");
      }
    }
  }
  if (logger) logger->set_pattern("[%Y-%m-%d %T.%e] [%n](%l) %v");

  // Check to see if we have an interface defined by env variable
  std::string interface;
  char const *iface_cstr = std::getenv("A17_DISPATCH_INTERFACE");
  if (iface_cstr != NULL) {
    interface = std::string(iface_cstr);
  } else {
    if (logger) logger->info("A17_DISPATCH_INTERFACE not specified, defaulting to localhost");
    return;
  }

  // Make sure at least one interface is available
  struct ifaddrs *ifaddr, *ifa;
  if (getifaddrs(&ifaddr) < 0) {
    if (logger) logger->error("Could not obtain iface addresses, defaulting to localhost");
    return;
  }

  std::string address, broadcast, netmask;
  // Iterate through each iface, if one matches our desired iface and it is INET, grab ip
  for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
    if (!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_INET) continue;
    if (strcmp(ifa->ifa_name, interface.c_str()) == 0) {
      if (getAddressFromSocket(address, ifa->ifa_addr) < 0) continue;
      if (getAddressFromSocket(broadcast, ifa->ifa_dstaddr) < 0) continue;
      if (getAddressFromSocket(netmask, ifa->ifa_netmask) < 0) continue;
      logger->trace("Attempting: {} | {} {} {}", interface, address, broadcast, netmask);
      // If we've populated our ip, broadcast and netmask addresses, we'll use them
      // TODO(pickledgator): check to see if they are valid
      if (!address.empty() && !broadcast.empty() && !netmask.empty()) {
        address_ = address;
        broadcast_ = broadcast;
        netmask_ = netmask;
        mask_ = ((sockaddr_in *)ifa->ifa_netmask)->sin_addr.s_addr;
        network_ = ((sockaddr_in *)ifa->ifa_addr)->sin_addr.s_addr & mask_;
        logger->info("Selected interface {} | ip {}", ifa->ifa_name, address_);
        logger->debug("Using bcast {} netmask {}", broadcast_, netmask_);
        interface_ = interface;
        break;
      }
    }
  }

  if (address.empty() && broadcast.empty() && netmask.empty()) {
    logger->warn("Could not obtain ip from iface: {}, defaulting to localhost", interface);
  }

  freeifaddrs(ifaddr);
}

bool OwnAddress::onNetwork(const std::string &address) {
  in_addr_t ip = inet_addr(address.c_str());
  return (ip & mask_) == network_;
}

Address Address::parse(const std::string &address) {
  std::string protocol("tcp");
  std::string ip;
  uint16_t port = 0;

  size_t pos = 0;
  size_t urlPos = address.find("://");
  if (urlPos != std::string::npos) {
    protocol = address.substr(0, urlPos);
    pos = urlPos + 3;
  }

  size_t portPos = address.find(":", pos);
  ip = address.substr(pos, portPos - pos);
  if (portPos != std::string::npos) {
    pos = portPos + 1;
    port = strtol(&address[pos], nullptr, 10);
  }

  return Address(protocol, ip, port);
}

}  // namespace dispatch
}  // namespace a17
