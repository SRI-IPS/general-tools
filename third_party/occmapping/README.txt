General Information
===================

Parts of the provided code are based on the original OctoMap implementation by
K.M. Wurm and A. Hornung. The code is licensed under the new BSD license,
which you find in the file LICENSE.txt.


Build Instructions
==================

For compiling the provided source code you require an installation of OctoMap
1.6.x (tested for 1.6.0 - 1.6.3), which can be obtained from:

http://octomap.github.io

The code can be compiled using the typical CMake build process:

> cmake .
> make


How to Use
==========

After compilation you will find a library in ./lib. All necessary header files
are located in ./inc.

The main class of this library is occmapping::RobustOcTree. Using this class
is equivalent to octomap::OcTree, for which you can find the documentation at:

http://octomap.github.io/octomap/doc/

The main differences are:

- Relevant parameters are set by passing an instance of
  occmapping::RobustOcTreeParameters to the constructor of
  occmapping::RobustOcTree. The default constructor of this parameter class
  already sets a reasonable set of default parameters.
- Ray casting always assumes a maximum range. This parameter is no longer
  passed as an argument to all ray casting methods, but is set in the
  parameter class.
- Change notifications and bound box limits are currently not supported.

For details on this method, please refer to:

Konstantin Schauwecker and Andreas Zell. Robust and Efficient Volumetric
Occupancy Mapping with an Application to Stereo Vision. IEEE International
Conference on Robotics and Automation (ICRA), May 2014


Contact
=======

If you have any questions please contact:

konstantin.schauwecker (at) uni-tuebingen.de

