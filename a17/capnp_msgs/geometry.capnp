@0x8cc820b55455e6ad;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("a17::capnp_msgs::geometry");

struct Bounds2d {
  min @0 :Point2d;
  max @1 :Point2d;
}

struct Cloud {
  enum SensorType {
    unknown @0;
    stereo @1;
    lidar2d @2;
    lidar3d @3;
    integrated3d @4;
  }
  timestamp @0 :UInt64;
  points @1 :List(CloudPoint);
  pose @2 :Pose;
  numPoints @3 :UInt16;
  sensor @4 :SensorType;
  sensorId @5 :UInt8;
}

struct CloudPoint {
  location @0 :Vector;
  depth @1 :Float64;
  numViews @2 :Int32;
}

enum CoordinateSystem {
  unknown @0;
  northEastDown @1;
  northWestUp @2;
  eastNorthUp @3;
}

struct OccupancyMap {
  resolution @0 :Float64;
  width @1 :UInt16;
  height @2 :UInt16;
  origin @3 :Vector;
  data @4 :List(Int8);
}

struct Octree {
  resolution @0 :Float64;
  binary @1 :Bool;
  data @2 :Data;
}

struct Path2d {
  points @0 :List(Vector2d);
}

struct Path2dStamped {
  timestamp @0 :UInt64;
  path @1 :Path2d;
}

struct Path3d {
  points @0 :List(Vector);
}

struct Path3dStamped {
  timestamp @0 :UInt64;
  path @1 :Path3d;
}

struct PathSegment {
  start @0 :Waypoint;
  end @1 :Waypoint;
  coefficients @2 :List(Float64);
  length @3 :Float64;
}

struct Point3d {
  x @0 :Float64;
  y @1 :Float64;
  z @2 :Float64;
}

struct Point2d {
  x @0 :Float64;
  y @1 :Float64;
}

struct PointInt3d {
  x @0 :Int32;
  y @1 :Int32;
  z @2 :Int32;
}

struct PointInt2d {
  x @0 :Int32;
  y @1 :Int32;
}

struct Pose {
  timestamp @0 :UInt64;
  position @1 :Vector;
  velocity @2 :Vector;
  quaternion @3 :Quaternion;
  quaternionRate @4 :Quaternion;
  rpy @5 :Vector;
  rpyRate @6 :Vector;
}

struct PoseWithCov {
  pose @0 :Pose;
  posCov @1 :Vector;
  rpyCov @2 :Vector;
}

struct Quaternion {
  w @0 :Float64;
  x @1 :Float64;
  y @2 :Float64;
  z @3 :Float64;
}

struct Vector {
  x @0 :Float64;
  y @1 :Float64;
  z @2 :Float64;
}

struct Vector2d {
  x @0 :Float64;
  y @1 :Float64;
}

struct VectorInt {
  x @0 :Int32;
  y @1 :Int32;
  z @2 :Int32;
}

struct VectorInt2d {
  x @0 :Int32;
  y @1 :Int32;
}

struct Size2d {
  width @0 :Float64;
  length @1 :Float64;
}

struct Size3d {
  width @0 :Float64;
  length @1 :Float64;
  height @2 :Float64;
}

struct Lumber {
  width @0 :Float64;
  length @1 :Float64;
  innerGap @2 :Float64;
}

struct Se2State {
  x @0 :Float64;
  y @1 :Float64;
  yaw @2 :Float64;
}

struct Se2Trajectory {
  waypoints @0 :List(Se2State);
}

# Next value: 7
struct Waypoint {
  x @0 :Float64;
  y @1 :Float64;
  z @2 :Float64;

  # A yaw angle in radians (rotation around z) that the drone is expected to be facing when it
  # reaches the given point. The fixedYaw flag must be set in order for this to have any effect.
  yaw @3 :Float64;

  # If true, then the given yaw will be used as the yaw for this waypoint. If false, the yaw will
  # be calculated automatically during flight.
  fixedYaw @5 :Bool;

  # If set, the path be guaranteed to get at least within this distance to the given point.
  radius @4 :Float64;

  # ID string identifier specific to this waypoint, used to track progress
  id @6 :Text;
}
