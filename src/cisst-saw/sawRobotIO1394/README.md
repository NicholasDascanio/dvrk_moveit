sawRobotIO1394
==============

This SAW component contains code for interfacing with the QLA (Quad
Linear Amplifier) via IEEE 1394 (FireWire), using the Amp1394 library.
This component is used in the da Vinci Research Kit.

Links
=====
 * License: http://github.com/jhu-cisst/cisst/blob/master/license.txt
 * JHU-LCSR software: http://jhu-lcsr.github.io/software/
 * QLA mechatronics and Amp1394 library: http://jhu-cisst.github.io/mechatronics
 * da Vinci Research Kit: http://github.com/jhu-dvrk

Dependencies
============
 * Amp1394 library requires `libraw1394` (Linux) or `pcap`/`WinPcap` (multi-platform, experimental)
 * cisst libraries: https://github.com/jhu-cisst/cisst

Building without cisst/saw
==========================

The core API ("osa" classes) can be built with or without CISST.
The core API has only a few dependencies on CISST, the largest of which is
cisstVector. If desired, the Eigen3 library can be used instead, by taking
advantage of Eigen's [customization
facilities](http://eigen.tuxfamily.org/dox-devel/TopicCustomizing_Plugins.html).
In this case, we are extending the `Eigen::Matrix` class by defining an
`EIGEN_MATRIX_PLUGIN` header file with some additional member functions which
make it compatible with cisstVector.

To build the core API without CISST, simply add the following definitions to a
CMakeLists.txt file which includes the appropriate source files:

```CMake
add_definitions(
  -DEIGEN_MATRIX_PLUGIN="sawRobotIO1394/cisstVectorEigenAddons.h"
  -DSAW_ROBOT_IO_1394_WO_CISST)
```
