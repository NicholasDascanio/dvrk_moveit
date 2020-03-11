/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*

  Author(s):  Kelleher Guerin, Simon Leonard
  Created on: 2011

  (C) Copyright 2011 Johns Hopkins University (JHU), All Rights
  Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---

*/

#ifndef _osaOpenNISkeleton_h
#define _osaOpenNISkeleton_h

#include <vector>
#include <cisstVector/vctFixedSizeVectorTypes.h>

class osaOpenNI;

// Always include last!
#include <sawOpenNI/sawOpenNIExport.h>

class CISST_EXPORT osaOpenNISkeleton {

private:

    osaOpenNI* OpenNI;


public:
    //! 3D Joint Coordinates
    std::vector<vct3> points3D;
    
	//! 2D Projective Coorinates
	std::vector<vctInt2> points2D;
    
	//! Joint Existance Confidence
	std::vector<bool> confidence;
    
	//! Skeleton has been populated coorectly
	bool exists;

	//! Default Constructor
	osaOpenNISkeleton(osaOpenNI * openNI);

	//! Default Deconstructor
	~osaOpenNISkeleton();

	//! Build Skeleton Using XN Context for a given user
	void Update(int id);
    void SetExists(bool val);
    void PrintUserState(void);
    void UpdateUserStates(void);
    std::vector<vct3> GetPoints3D(void);
    
    int usrState;
    int calState;
    int skelID;

};

#endif
