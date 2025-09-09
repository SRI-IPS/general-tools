@0xc957c0f443fc300f;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("a17::capnp_msgs::error");

struct Error
{
  enum Status {
    error @0;
    noerror   @1;
  }
  timestamp     @0 : UInt64;
  result        @1 : Status;
  message       @2 : Text;
}
