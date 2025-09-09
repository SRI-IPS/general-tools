@0x819d24eef13b780c;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("a17::capnp_msgs::test");

using Geometry = import "geometry.capnp";

struct TestType
{
  timestamp   @0 : UInt64;
  val         @1 : Float64;
  position    @2 : Geometry.Vector;
  alist       @3 : List(Int32);
  vectorList  @4 : List(Geometry.Vector);
}

struct DispatchTest
{
  enum Event
  {
    send @0;
    receive @1;
  }

  topic       @0 : Text;
  socketType  @1 : Text;
  address     @2 : Text;
  event       @3 : Event;
}
