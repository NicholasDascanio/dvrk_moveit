/*

  Author(s): Simon Leonard
  Created on: Nov 11 2009

  (C) Copyright 2009-2011 Johns Hopkins University (JHU), All Rights Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---
*/

#ifndef _osaGLUT_h
#define _osaGLUT_h

#include <stdlib.h>
#include <vector>
#include <cisstVector/vctFixedSizeVectorTypes.h>

#include <sawGLUTSimulator/sawGLUTSimulatorExport.h>

class osaGeometry;

class CISST_EXPORT osaGLUT {

 private:

  int x, y;           // X, Y positions (top left corner)
  int width, height;  // width and heiht
    
  double azimuth;       // rotation about the Z axis
  double elevation;     // elevation from the X-Y plane
  double distance;      // distance from the origin
  float perspective;  // camera FOV

  double sleepPeriod;  // to save CPU
  
  void DrawXYZ();     // draw the X-Y-Z axis
  void DrawGrid(double width, int subdivisions); // draw the floor
  
  // compute the camera XYZ coordinates
  vct3 CameraPosition() const;
  
  std::vector<const osaGeometry*> geoms; // the geometries
  
 public:

  static osaGLUT* glut;
  
  //! Default constructor
  osaGLUT( int argc, char** argv);
  
  //! 
  static void Register( const osaGeometry* geom );

  //! Calls  glutMainLoop (blocking call)
  static void StartMainLoop(void);

  //! Draw everything
  void Draw();
  
  //! Process the keboard
  void Keyboard( int k, int x, int y );

  osaGeometry* LoadOBJ( const std::string& filename );

  //! Setter
  void SetSleepPeriod( const double sleepPeriodInSec ) {
      sleepPeriod = sleepPeriodInSec;
  }

  static void Refresh();
};

#endif
