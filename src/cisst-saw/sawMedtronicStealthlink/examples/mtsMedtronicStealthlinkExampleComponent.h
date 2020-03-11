/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*

  Author(s):  Peter Kazanzides, Anton Deguet
  Created on: 2011-07-14

  (C) Copyright 2011-2012 Johns Hopkins University (JHU), All Rights Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---
*/

#include <cisstMultiTask/mtsTaskPeriodic.h>

#ifndef _mtsMedtronicStealthlinkExampleComponent_h
#define _mtsMedtronicStealthlinkExampleComponent_h

// define to not use any medtronic specific payloads (defined in mtsMedtronicStealthlinkTypes)
// #define SAW_MEDTRONIC_WITHOUT_STEALTHLINK_TYPES

class mtsMedtronicStealthlinkExampleComponent: public mtsTaskPeriodic
{
public:

    struct ControllerStruct {
        mtsFunctionRead GetTool;
        mtsFunctionRead GetFrame;
    };

    struct ToolStruct {
        mtsFunctionRead GetMarkerCartesian;
        mtsFunctionRead GetPositionCartesian;
    };

    struct RegistrationStruct {
        mtsFunctionRead GetTransformation;
        mtsFunctionRead GetPredictedAccuracy;
        mtsFunctionRead GetValid;
    };

    struct ExamInformationStruct {
        mtsFunctionVoid RequestExamInformation;
        mtsFunctionRead GetVoxelScale;
        mtsFunctionRead GetSize;
        mtsFunctionRead GetValid;
    };

    struct CollectorStateStruct {
        mtsFunctionVoid StartCollection;
        mtsFunctionVoid StopCollection;
    };

    struct TestComponentStruct{
        mtsFunctionVoid StateTableAdvance;
    };

    void BatchReadyHandler(const mtsStateTable::IndexRange & range) {
        this->BatchReadyEventCounter++;
        this->LastRange = range;
    }

    void CollectionStartedHandler(void) {
        this->CollectionRunning = true;
    }

    void CollectionStoppedHandler(const mtsUInt & samplesCollected) {
        this->CollectionRunning = false;
        this->SamplesCollected = samplesCollected;
    }

    mtsMedtronicStealthlinkExampleComponent(const std::string & name,
                                            double periodInSeconds);

    mtsMedtronicStealthlinkExampleComponent(const std::string & name,
                                            double periodInSeconds,
                                            bool enableStateCollectionInterface);

    ~mtsMedtronicStealthlinkExampleComponent() {}

    void Run(void);

    ControllerStruct Stealthlink;
    ToolStruct Pointer;
    ToolStruct Frame;
    RegistrationStruct Registration;
    ExamInformationStruct ExamInformation;

    unsigned int BatchReadyEventCounter; // counter for range events from state table
    mtsStateTable::IndexRange LastRange;
    bool CollectionRunning;
    unsigned int SamplesCollected;
    CollectorStateStruct CollectorState;
    TestComponentStruct TestComponent;

protected:
    void AddStealthlinkInterface();

    void AddStateCollectionInterface();

    void AddToolInterface(const std::string & toolName,
                          ToolStruct & functionSet);
};

#endif // _mtsMedtronicStealthlinkExampleComponent_h
