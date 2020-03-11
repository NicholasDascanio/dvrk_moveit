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

#include <sawCANBus/mtsCANBusFrame.h>
#include <cisstCommon/cmnLogger.h>

CMN_IMPLEMENT_SERVICES( mtsCANBusFrame )

mtsCANBusFrame::mtsCANBusFrame() : 
  mtsGenericObject(),
  osaCANBusFrame(){}

mtsCANBusFrame::mtsCANBusFrame( const osaCANBusFrame& frame ) :
  mtsGenericObject(),
  osaCANBusFrame( frame ){}

mtsCANBusFrame::mtsCANBusFrame( ID canid, DataField data, DataLength nbytes ) : 
  mtsGenericObject(),
  osaCANBusFrame( canid, data, nbytes ){}

mtsCANBusFrame::mtsCANBusFrame( ID canid, 
				const vctDynamicVector<osaCANBusFrame::Data>& data ): 
  mtsGenericObject(),
  osaCANBusFrame( canid, data ){}

