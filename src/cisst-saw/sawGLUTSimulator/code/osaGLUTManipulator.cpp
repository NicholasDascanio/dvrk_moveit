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

#include <sawGLUTSimulator/osaGLUTManipulator.h>
#include <sawGLUTSimulator/osaGLUT.h>

#include "osaMeshTriangular.h"

osaGLUTManipulator::osaGLUTManipulator(const std::vector<std::string>& geomfiles,
                      const vctFrame4x4<double>& Rtw0,
                      const std::string& robotfn,
                      const vctDoubleVec& qinit,
                      const std::string& basefile,
                      bool rotateX90) :
  robManipulator(robotfn, Rtw0),
  q(qinit),
  base(0)
{
    Initialize(geomfiles, basefile, rotateX90);
}

osaGLUTManipulator::osaGLUTManipulator(const std::vector<std::string>& geomfiles,
                      const vctFrm3& Rtw0,
                      const std::string& robotfn,
                      const vctDoubleVec& qinit,
                      const std::string& basefile,
                      bool rotateX90) :
  robManipulator(robotfn, 
                 vctFrame4x4<double>(Rtw0.Rotation(), Rtw0.Translation())),
  q(qinit),
  base(0)
{
    Initialize(geomfiles, basefile, rotateX90);
}

osaGLUTManipulator::~osaGLUTManipulator() {}

void osaGLUTManipulator::Initialize(const std::vector<std::string>& geomfiles,
                                    const std::string& basefile, bool rotateX90)
{
    if( !basefile.empty() ){
        base = new osaMeshTriangular();
        base->LoadOBJ( basefile, rotateX90 );
        osaGLUT::Register( base );
    }

    // create the links and add them to the link group
    for( size_t i=0; i<links.size(); i++ ){
        osaMeshTriangular* mesh = new osaMeshTriangular();
        mesh->LoadOBJ( geomfiles[i], rotateX90 );
        osaGLUT::Register( mesh );
        meshes.push_back( mesh );
   }
}

bool osaGLUTManipulator::GetPositions(vctDoubleVec& q) const
{
    q = this->q; 
    return true;
}

bool osaGLUTManipulator::SetPositions(const vctDoubleVec& q)
{ 
    // Ensure one joint value per link
    if (q.size() != links.size()) {
        CMN_LOG_RUN_ERROR << "osaGLUTManipulator::SetPositions: expected "
                          << links.size() << " values, got "
                          << q.size() << " values." << std::endl;
        return false;
    }


    this->q = q;
    for( size_t i=0; i < meshes.size(); i++ ){
      vctFrame4x4<double> Rtwi = ForwardKinematics( q, i+1 );
      meshes[i]->SetPositionOrientation( Rtwi );
    }
    return true;
}
