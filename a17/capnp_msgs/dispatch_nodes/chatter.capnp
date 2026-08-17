@0xb5f2a8e03f7c1a20;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("a17::capnp_msgs::dispatch_nodes::chatter");

struct Chatter {
  timestamp @0 :UInt64;
  sender    @1 :Text;
  message   @2 :Text;
}
