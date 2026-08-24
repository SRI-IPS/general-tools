#include <iostream>
#include <string>

#include "a17/dispatch/node.h"
#include "a17/capnp_msgs/dispatch_nodes/chatter.capnp.h"

int main(int argc, const char *argv[]) {
  std::string node_name = "TALKER";
  std::string device_name;

  // Simple argument parsing
  for (int i = 1; i < argc; i++) {
    std::string arg(argv[i]);
    if (arg == "--node-name" && i + 1 < argc) {
      node_name = argv[++i];
    } else if (arg == "--device-name" && i + 1 < argc) {
      device_name = argv[++i];
    } else if (arg == "--help" || arg == "-h") {
      std::cout << "Usage: dispatch_talker [options]" << std::endl;
      std::cout << "  --node-name NAME    Node name (default: TALKER)" << std::endl;
      std::cout << "  --device-name NAME  Device name prefix for topics" << std::endl;
      return 0;
    }
  }

  if (!device_name.empty()) {
    setenv("A17_DEVICE_NAME", device_name.c_str(), 1);
  }

  a17::dispatch::Node node(node_name);
  auto topic = node.topic("CHATTER");
  auto pub = node.registerCapnpPublisher<a17::capnp_msgs::dispatch_nodes::chatter::Chatter>(topic);

  int count = 0;
  auto repeater = node.registerRepeater(100, [&]() -> bool {
    count++;
    auto builder = node.newCapnpMessageBuilder();
    auto msg = builder.initRoot<a17::capnp_msgs::dispatch_nodes::chatter::Chatter>();
    msg.setTimestamp(static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now().time_since_epoch())
            .count()));
    msg.setSender(node_name.c_str());
    msg.setMessage(("Hello world #" + std::to_string(count)).c_str());

    auto ec = pub->send(builder);
    if (ec) {
      std::cerr << "[talker] send error: " << strerror(ec.value()) << std::endl;
    } else {
      std::cout << "[talker] published message #" << count << std::endl;
    }

    return true;
  });

  std::cout << "[talker] publishing on topic: " << topic.str() << std::endl;
  node.run();

  return 0;
}
