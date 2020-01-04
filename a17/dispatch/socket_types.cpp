#include "socket_types.h"

// From zmq:
// ZMQ_PAIR 0
// ZMQ_PUB 1
// ZMQ_SUB 2
// ZMQ_REQ 3
// ZMQ_REP 4
// ZMQ_DEALER 5
// ZMQ_ROUTER 6
// ZMQ_PULL 7
// ZMQ_PUSH 8
// ZMQ_XPUB 9
// ZMQ_XSUB 10
// ZMQ_STREAM 11

namespace a17 {
namespace dispatch {

const SocketTypes SocketTypes::instance;

SocketTypes::SocketTypes()
    : types{"PAIR",   "PUB",  "SUB",  "REQ",  "REP",  "DEALER",
            "ROUTER", "PULL", "PUSH", "XPUB", "XSUB", "STREAM"} {
  for (int i = 0; i < static_cast<int>(types.size()); i++) {
    byName[types[i]] = i;
  }

  connect(ZMQ_PAIR, {ZMQ_PAIR});
  connect(ZMQ_PUB, {ZMQ_SUB});
  connect(ZMQ_SUB, {ZMQ_PUB});
  connect(ZMQ_REQ, {ZMQ_REP});
  connect(ZMQ_REP, {ZMQ_REQ});
  connect(ZMQ_DEALER, {ZMQ_ROUTER});
  connect(ZMQ_ROUTER, {ZMQ_DEALER});
  connect(ZMQ_PULL, {ZMQ_PUSH});
  connect(ZMQ_PUSH, {ZMQ_PULL});
  connect(ZMQ_XPUB, {ZMQ_XSUB});
  connect(ZMQ_XSUB, {ZMQ_XPUB});
  connect(ZMQ_STREAM, {ZMQ_STREAM});
}

void SocketTypes::connect(int type, const std::set<int> &connect) { connectable[type] = connect; }

bool SocketTypes::canConnect(int from, int to) const {
  const std::set<int> &types = connectable.at(from);
  return types.find(to) != types.end();
}

}  // namespace dispatch
}  // namespace a17
