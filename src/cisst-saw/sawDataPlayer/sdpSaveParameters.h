/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*

  Author(s): Marcin Balicki
  Created on: 2011-02-10

  (C) Copyright 2011 Johns Hopkins University (JHU), All Rights
  Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---
*/

/*!
\file
\brief Container for save parameters.
*/


#ifndef _sdpSaveParameters_h
#define _sdpSaveParameters_h

#include <cisstMultiTask/mtsGenericObject.h>
#include <cisstMultiTask/mtsGenericObjectProxy.h>


class sdpSaveParameters: public mtsGenericObject
{
    CMN_DECLARE_SERVICES(CMN_DYNAMIC_CREATION, CMN_LOG_LOD_RUN_ERROR);

public:
    typedef sdpSaveParameters ThisType;

    /*! default constructor - does nothing for now */
    sdpSaveParameters();

    /*! Copy constructor. */
    inline sdpSaveParameters(const ThisType & other):
        mtsGenericObject( other ),
        StartMember( other.Start() ),
        EndMember( other.End() ),
        PathMember( other.Path() ),
        PrefixMember( other.Prefix() )
    {}


    /*! Human readable output to stream. */
    void ToStream(std::ostream & outputStream) const;

    /*! destructor */
    ~sdpSaveParameters() {};

    /*! Set and Get methods for the values */
    //@{
    MTS_DECLARE_MEMBER_AND_ACCESSORS(double, Start);
    //@}

    /*! Set and Get methods Mask. True indicates an existting force/torque axis  */
    //@{
    MTS_DECLARE_MEMBER_AND_ACCESSORS(double, End);
    //@}

    /*! Set and Get methods for IsSaturated vector  */
    //@{
    MTS_DECLARE_MEMBER_AND_ACCESSORS(mtsStdString, Path);
    //@}

    /*!  */
    //@{
    MTS_DECLARE_MEMBER_AND_ACCESSORS(mtsStdString, Prefix);
    //@}


};

CMN_DECLARE_SERVICES_INSTANTIATION(sdpSaveParameters);

#endif // _sdpSaveParameters_h
