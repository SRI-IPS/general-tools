#include <iostream>
#include <string>

#include "a17/dispatch/node.h"
#include "a17/capnp_msgs/dispatch_nodes/chatter.capnp.h"

int main(int argc, const char *argv[]) {
  std::string node_name = "LISTENER";
  std::string device_name;

  // Simple argument parsing
  for (int i = 1; i < argc; i++) {
    std::string arg(argv[i]);
    if (arg == "--node-name" && i + 1 < argc) {
      node_name = argv[++i];
    } else if (arg == "--device-name" && i + 1 < argc) {
      device_name = argv[++i];
    } else if (arg == "--help" || arg == "-h") {
      std::cout << "Usage: dispatch_listener [options]" << std::endl;
      std::cout << "  --node-name NAME    Node name (default: LISTENER)" << std::endl;
      std::cout << "  --device-name NAME  Device name prefix for topics" << std::endl;
      return 0;
    }
  }

  if (!device_name.empty()) {
    setenv("A17_DEVICE_NAME", device_name.c_str(), 1);
  }

  a17::dispatch::Node node(node_name);
  auto topic = node.topic("CHATTER");

  auto sub = node.registerCapnpSubscriber<
      a17::capnp_msgs::dispatch_nodes::chatter::Chatter>(
      topic,
      [](const a17::capnp_msgs::dispatch_nodes::chatter::Chatter::Reader &msg) {
        std::cout << "[listener] received: sender="
                  << msg.getSender().cStr()
                  << " message=" << msg.getMessage().cStr()
                  << " timestamp=" << msg.getTimestamp()
                  << std::endl;
      });

  std::cout << "[listener] listening on topic: " << topic.str() << std::endl;
  node.run();

  return 0;
}
