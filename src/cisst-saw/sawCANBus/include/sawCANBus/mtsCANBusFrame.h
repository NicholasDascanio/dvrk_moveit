/*

  Author(s): Simon Leonard
  Created on: Dec 02 2009

  (C) Copyright 2009 Johns Hopkins University (JHU), All Rights
  Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---
*/


#ifndef _mtsCANBusFrame_h
#define _mtsCANBusFrame_h

#include <cisstMultiTask/mtsGenericObject.h>
#include <sawCANBus/osaCANBusFrame.h>
#include <sawCANBus/sawCANBusExport.h>

class CISST_EXPORT mtsCANBusFrame : 

  public mtsGenericObject,
  public osaCANBusFrame {
  
  CMN_DECLARE_SERVICES( CMN_NO_DYNAMIC_CREATION, CMN_LOG_LOD_RUN_ERROR );
  
 public:

  //! Create an empty CAN frame
  mtsCANBusFrame();

  //! Create a mtsCANBusFrame from a osaCANBusFrame
  mtsCANBusFrame( const osaCANBusFrame& frame );

  //! Create a mtsCANBusFrame from a CAN id and data field
  mtsCANBusFrame( ID canid, DataField data, DataLength nbytes );
  
  //! Create a mtsCANBusFrame from a vctVector
  mtsCANBusFrame( ID canid, const vctDynamicVector<osaCANBusFrame::Data>& data );

};

CMN_DECLARE_SERVICES_INSTANTIATION( mtsCANBusFrame )

#endif
