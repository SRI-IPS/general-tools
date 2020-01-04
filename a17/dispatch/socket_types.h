#pragma once

#include <zmq.h>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace a17 {
namespace dispatch {

class SocketTypes {
 public:
  SocketTypes();
  int fromName(const std::string &name) const { return byName.count(name) ? byName.at(name) : -1; }
  const std::string &fromType(int type) const { return operator[](type); }
  const std::string &operator[](int type) const { return types[type]; }
  bool canConnect(int from, int to) const;

  static const SocketTypes instance;

 private:
  void connect(int type, const std::set<int> &connect);

  std::vector<std::string> types;
  std::map<int, std::string> byType;
  std::map<std::string, int> byName;
  std::map<int, std::set<int>> connectable;
  std::string none;
};

}  // namespace dispatch
}  // namespace a17
