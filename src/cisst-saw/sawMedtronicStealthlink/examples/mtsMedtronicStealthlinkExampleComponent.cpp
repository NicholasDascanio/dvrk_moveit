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

#include "mtsMedtronicStealthlinkExampleComponent.h"
#include <cisstMultiTask/mtsInterfaceRequired.h>
#include <cisstMultiTask/mtsTransformationTypes.h>
#include <cisstParameterTypes/prmPositionCartesianGet.h>

#ifndef SAW_MEDTRONIC_WITHOUT_STEALTHLINK_TYPES
    #ifdef SAW_MEDTRONIC_STEALTHLINK2
        #include <sawMedtronicStealthlink/mtsMedtronicStealthlink2Types.h>
    #else
        #include <sawMedtronicStealthlink/mtsMedtronicStealthlinkTypes.h>
    #endif
#endif

#include <cisstMultiTask/mtsFixedSizeVectorTypes.h>

mtsMedtronicStealthlinkExampleComponent::mtsMedtronicStealthlinkExampleComponent(const std::string & name,
                                                                                 double periodInSeconds):
    mtsTaskPeriodic(name, periodInSeconds),
    BatchReadyEventCounter(0),
    CollectionRunning(false),
    SamplesCollected(0)
{

   AddStealthlinkInterface();
}

mtsMedtronicStealthlinkExampleComponent::mtsMedtronicStealthlinkExampleComponent(const std::string & name,
                                                                                 double periodInSeconds,
                                                                                 bool enableStateCollectionInterface):
    mtsTaskPeriodic(name, periodInSeconds),
    BatchReadyEventCounter(0),
    CollectionRunning(false),
    SamplesCollected(0)
{

    AddStealthlinkInterface();

    //state collection
    if(enableStateCollectionInterface)
        AddStateCollectionInterface();
}

void mtsMedtronicStealthlinkExampleComponent::AddStealthlinkInterface()
{
    mtsInterfaceRequired * required = AddInterfaceRequired("Stealthlink");
    if (required) {
#ifndef SAW_MEDTRONIC_WITHOUT_STEALTHLINK_TYPES
        required->AddFunction("GetTool", Stealthlink.GetTool);
        required->AddFunction("GetFrame", Stealthlink.GetFrame);
#endif
    }

    // the following two tools are using names normally defined in config.xml
    AddToolInterface("Pointer", Pointer);
    AddToolInterface("Frame", Frame);

    // get registration information
    required = AddInterfaceRequired("Registration");
    if (required) {
        required->AddFunction("GetTransformation", Registration.GetTransformation);
        required->AddFunction("GetPredictedAccuracy", Registration.GetPredictedAccuracy);
        required->AddFunction("GetValid", Registration.GetValid);
    }

    // get exam information
    required = AddInterfaceRequired("ExamInformation");
    if (required) {
        required->AddFunction("RequestExamInformation", ExamInformation.RequestExamInformation);
        required->AddFunction("GetVoxelScale", ExamInformation.GetVoxelScale);
        required->AddFunction("GetSize", ExamInformation.GetSize);
        required->AddFunction("GetValid", ExamInformation.GetValid);
    }
}

void mtsMedtronicStealthlinkExampleComponent::AddStateCollectionInterface()
{
    mtsInterfaceRequired * required;

    required = AddInterfaceRequired("CollectorState");
    if (required) {
        required->AddFunction("StartCollection", CollectorState.StartCollection);
        required->AddFunction("StopCollection", CollectorState.StopCollection);
    }
}

void mtsMedtronicStealthlinkExampleComponent::AddToolInterface(const std::string & toolName,
                                                               mtsMedtronicStealthlinkExampleComponent::ToolStruct & functionSet)
{
    mtsInterfaceRequired * required = AddInterfaceRequired(toolName, MTS_OPTIONAL);
    if (required) {
        required->AddFunction("GetPositionCartesian", functionSet.GetPositionCartesian);
        required->AddFunction("GetMarkerCartesian", functionSet.GetMarkerCartesian);
    }
}


void mtsMedtronicStealthlinkExampleComponent::Run(void)
{
#ifndef SAW_MEDTRONIC_WITHOUT_STEALTHLINK_TYPES
    mtsStealthTool StealthTool;
    mtsStealthFrame StealthFrame;
#endif
    prmPositionCartesianGet prmPos;
    mtsFrm3 mtsFrm;
    mtsDouble predictedAccuracy;

    bool debug = true;
    bool didOutput = false;
    bool getValid = false;
    mtsExecutionResult result;
#ifndef SAW_MEDTRONIC_WITHOUT_STEALTHLINK_TYPES
    bool getValid = false;
    result = Stealthlink.GetTool(StealthTool);
    if (!result.IsOK()) {
        std::cout << "Stealthlink.GetTool() failed: " << result << std::endl;
    }
    StealthTool.GetValid(getValid);
    if (!getValid) {
        std::cout << "Stealthlink.GetTool().GetValid() failed: " << getValid << std::endl;
    }

    if (StealthTool.Valid()) {
        if(debug)
        {
            std::cout << "Tool " << StealthTool.GetName() << ": " << StealthTool.GetFrame().Translation() << "; ";
            std::cout << StealthTool.GetFrame().Rotation() << "; ";
        }
        didOutput = true;
    }else
        std::cout << "Stealthlink.Valid() FAILED: " << StealthTool.Valid() << std::endl;

    result = Stealthlink.GetFrame(StealthFrame);
    if (!result.IsOK()) {
        std::cout << "Stealthlink.GetFrame() failed: " << result << std::endl;
    }
    StealthFrame.GetValid(getValid);
    if (!getValid) {
        std::cout << "Stealthlink.GetFrame().GetValid failed: " << getValid << std::endl;
    }

    if (StealthFrame.Valid()) {
        if(debug)
            std::cout << "Frame " << StealthFrame.GetName() << ": " << StealthFrame.GetFrame().Translation() << "; ";
        didOutput = true;
    }else
        std::cout << "Stealthlink.GetFrame().Valid() FAILED: " << StealthTool.Valid() << std::endl;
#endif // SAW_MEDTRONIC_WITHOUT_STEALTHLINK_TYPES

    if (Pointer.GetPositionCartesian.IsValid()) {
        result = Pointer.GetPositionCartesian(prmPos);
        if (!result.IsOK()) {
            std::cout << "Pointer.GetPositionCartesian() failed: " << result << std::endl;
        }
        if (prmPos.Valid()) {
            if(debug)
                std::cout << "Interface Pointer: " << prmPos.Position().Translation() << "; ";
            didOutput = true;
        }
    }
    if (Pointer.GetMarkerCartesian.IsValid()) {
        result = Pointer.GetMarkerCartesian(prmPos);
        if (!result.IsOK()) {
            std::cout << "Pointer.GetMarkerCartesian() failed: " << result << std::endl;
        }
        if (prmPos.Valid()) {
            if(debug)
                std::cout << "Interface PointerM: " << prmPos.Position().Translation() << "; ";
            didOutput = true;
        }
    }

    if (Frame.GetPositionCartesian.IsValid()) {
        result = Frame.GetPositionCartesian(prmPos);
        if (!result.IsOK()) {
            std::cout << "Frame.GetPositionCartesian() failed: " << result << std::endl;
        }
        if (prmPos.Valid()) {
            if(debug)
                std::cout << "Interface Frame: " << prmPos.Position().Translation() << "; ";
            didOutput = true;
        } else {
            if(debug)
                std::cerr << "Interface Frame, invalid position" << std::endl;
            didOutput = true;
        }
    }
    if (Frame.GetMarkerCartesian.IsValid()) {
        result = Frame.GetMarkerCartesian(prmPos);
        if (!result.IsOK()) {
            std::cout << "Frame.GetMarkerCartesian() failed: " << result << std::endl;
        }
        if (prmPos.Valid()) {
            if(debug)
                std::cout << "Interface FrameM: " << prmPos.Position().Translation() << "; ";;
            didOutput = true;
        } else {
            if(debug)
                std::cerr << "Interface Frame, invalid position" << std::endl;
            didOutput = true;
        }
    }

    mtsBool valid;
    if (Registration.GetValid.IsValid()) {
        result = Registration.GetValid(valid);
        if (!result.IsOK()) {
            std::cout << "Registration.GetValid() failed: " << result << std::endl;
        }
        if (valid) {
            Registration.GetTransformation(mtsFrm);
            if(debug)
                std::cout << "Registration: " << mtsFrm.Translation() << "; ";;
            didOutput = true;
        } else {
            if(debug)
                std::cout << "Registration: invalid" << std::endl;
            didOutput = true;
        }
    }

    if (ExamInformation.RequestExamInformation.IsValid()) {
        result = ExamInformation.RequestExamInformation.ExecuteBlocking();
        if (!result.IsOK()) {
            std::cout << "ExamInformation.RequestExamInformation() failed: " << result << std::endl;
        } else {
            result = ExamInformation.GetValid(valid);
            if (!result.IsOK()) {
                std::cout << "ExamInformation.GetValid() failed: " << result << std::endl;
            } else {
                if (valid) {
                    mtsDouble3 voxelScale;
                    result = ExamInformation.GetVoxelScale(voxelScale);
                    if (!result.IsOK()) {
                        std::cout << "ExamInformation.GetVoxelScale() failed: " << result << std::endl;
                    } else {
                        if(debug)
                            std::cout << "Voxel scale: " << voxelScale << "; ";
                        didOutput = true;
                    }
                    mtsInt3 sizes;
                    result = ExamInformation.GetSize(sizes);
                    if (!result.IsOK()) {
                        if(debug)
                            std::cout << "ExamInformation.GetSize() failed: " << result << std::endl;
                    } else {
                        if(debug)
                            std::cout << "Size: " << sizes << "; ";
                    }
                } else {
                    if(debug)
                        std::cout << "ExamInformation is not valid" << std::endl;
                    didOutput = true;
                }
            }
        }
    }

    if (didOutput && debug) {
        std::cout << std::endl;
    } else {
        if(debug)
            std::cout << "." << std::flush;
    }
}
