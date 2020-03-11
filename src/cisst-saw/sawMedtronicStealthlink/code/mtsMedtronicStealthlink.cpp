/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*

  Author(s): Peter Kazanzides, Anton Deguet
  Created on: 2006

  (C) Copyright 2007-2011 Johns Hopkins University (JHU), All Rights Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---
*/



// Stealthlink definitions
#include <sawMedtronicStealthlink/mtsMedtronicStealthlink.h>

#include <cisstCommon/cmnPortability.h>
#include <cisstCommon/cmnXMLPath.h>
#include <cisstMultiTask/mtsFixedSizeVectorTypes.h>
#include <cisstMultiTask/mtsInterfaceProvided.h>

#ifdef CISST_HAS_STEALTHLINK
#if (CISST_OS == CISST_WINDOWS)
#include <GRI.h>
#else
#include <GRI_Protocol/GRI.h>
#endif
#endif

#ifdef CISST_HAS_STEALTHLINK
#if (CISST_OS == CISST_WINDOWS)
// Prevent inclusion of <winsock.h> from <windows.h>.
#define _WINSOCKAPI_
#endif
#endif

// Stealthlink include files
//#ifdef CISST_HAS_STEALTHLINK
#if (CISST_OS == CISST_WINDOWS)
#include <AsCL_Client.h>
#else
#include <AsCL/AsCL_Client.h>
#endif
//#endif

#include "mtsMedtronicStealthlink_AsCL_Stuff.h"

#ifdef sawMedtronicStealthlink_IS_SIMULATOR
void AsCL_MSG(int CMN_UNUSED(verbose_level), char * CMN_UNUSED(msg), ...) {}
#endif

CMN_IMPLEMENT_SERVICES_DERIVED_ONEARG(mtsMedtronicStealthlink, mtsTaskPeriodic, mtsTaskPeriodicConstructorArg)

void mtsMedtronicStealthlink::Init(void)
{
    // create Stealthlink objects
#ifdef sawMedtronicStealthlink_IS_SIMULATOR
    this->Client = 0;
#else
    this->Client = new AsCL_Client;
#endif
    this->Utils = new mtsMedtronicStealthlink_AsCL_Utils;

    TrackMultTools = false;

    SurgicalPlan.SetSize(6);

    // Stealth Tool -- the position of the tracked tool, as a frame
    StateTable.AddData(ToolData, "ToolData");
    // Stealth Frame -- the position of the base frame, as a frame
    StateTable.AddData(FrameData, "FrameData");
    // Stealth Registration
    StateTable.AddData(RegistrationData, "RegistrationData");
    // Stealth Tool Calibration
    StateTable.AddData(ProbeCal, "ProbeCalibration");

    mtsInterfaceProvided * provided = AddInterfaceProvided("Controller");
    if (provided) {
        provided->AddCommandReadState(StateTable, ToolData, "GetTool");
        provided->AddCommandReadState(StateTable, FrameData, "GetFrame");
        provided->AddCommandReadState(StateTable, RegistrationData, "GetRegistration");
        provided->AddCommandReadState(StateTable, ProbeCal, "GetProbeCalibration");
        provided->AddCommandVoid(&mtsMedtronicStealthlink::RequestSurgicalPlan, this, "RequestSurgicalPlan");
        provided->AddCommandRead(&mtsMedtronicStealthlink::GetSurgicalPlan, this, "GetSurgicalPlan", SurgicalPlan);
    }

    // Add interface for registration, ideally we should standardize such interface commands/payloads
    // maybe we should create a separate state table for registration?  Would only advance if changed.
    StateTable.AddData(RegistrationMember.Transformation, "RegistrationTransformation");
    StateTable.AddData(RegistrationMember.Valid, "RegistrationValid");
    StateTable.AddData(RegistrationMember.PredictedAccuracy, "RegistrationPredictedAccuracy");
    provided = AddInterfaceProvided("Registration");
    if (provided) {
        provided->AddCommandReadState(StateTable, RegistrationMember.Transformation, "GetTransformation");
        provided->AddCommandReadState(StateTable, RegistrationMember.Valid, "GetValid");
        provided->AddCommandReadState(StateTable, RegistrationMember.PredictedAccuracy, "GetPredictedAccuracy");
    }

    // Add interface for exam information
    StateTable.AddData(ExamInformationMember.VoxelScale, "ExamInformationVoxelScale");
    StateTable.AddData(ExamInformationMember.Size, "ExamInformationSize");
    StateTable.AddData(ExamInformationMember.Valid, "ExamInformationValid");
    provided = AddInterfaceProvided("ExamInformation");
    if (provided) {
        provided->AddCommandVoid(&mtsMedtronicStealthlink::RequestExamInformation, this, "RequestExamInformation");
        provided->AddCommandReadState(StateTable, ExamInformationMember.VoxelScale, "GetVoxelScale");
        provided->AddCommandReadState(StateTable, ExamInformationMember.Size, "GetSize");
        provided->AddCommandReadState(StateTable, ExamInformationMember.Valid, "GetValid");
    }
}

mtsMedtronicStealthlink::mtsMedtronicStealthlink(const std::string & taskName, const double & periodInSeconds) :
    mtsTaskPeriodic(taskName, periodInSeconds, false, 1000),
    StealthlinkPresent(false),
    CurrentTool(0),
    CurrentFrame(0),
    CurrentMultTool {0, 0, 0, 0, 0}
{
    Init();
}


mtsMedtronicStealthlink::mtsMedtronicStealthlink(const mtsTaskPeriodicConstructorArg &arg) :
    mtsTaskPeriodic(arg),
    StealthlinkPresent(false),
    CurrentTool(0),
    CurrentFrame(0),
    CurrentMultTool {0, 0, 0, 0, 0}
{
    Init();
}


mtsMedtronicStealthlink::~mtsMedtronicStealthlink()
{
    Cleanup();
}


// Windows defines a SetPort macro
#ifdef SetPort
#undef SetPort
#endif

void mtsMedtronicStealthlink::Configure(const std::string &filename)
{
    CMN_LOG_CLASS_INIT_VERBOSE << "Configure: using " << filename << std::endl;

    cmnXMLPath config;
    config.SetInputSource(filename);

    // initialize serial port
    std::string ipAddress;
    if (!config.GetXMLValue("/tracker/controller", "@ip", ipAddress, "192.168.0.1")) {
        CMN_LOG_CLASS_INIT_WARNING << "Configure: IP address not found, using default: " << ipAddress << std::endl;
    }

#ifndef sawMedtronicStealthlink_IS_SIMULATOR
    // Configure MedtronicStealthlink interface
    AsCL_SetVerboseLevel(0); //not in SL2
    this->Client->SetPort(GRI_PORT_NUMBER);

    // Set StealthLink server IP address
    CMN_LOG_CLASS_INIT_VERBOSE << "Configure: setting Stealthink IP address = " << ipAddress << std::endl;
    this->Client->SetHostName(const_cast<char *>(ipAddress.c_str()));
#endif

    // Set Tracking Tool mode (MultTools or SingleTool)
    std::string trackMultTools;
    if (config.GetXMLValue("/tracker/trackmode", "@mode", trackMultTools, "")) {
        if ( trackMultTools.compare("MultTools") == 0 ) {
            TrackMultTools = true;
        } else {
            TrackMultTools = false;
        }

        CMN_LOG_CLASS_INIT_VERBOSE << "Configure: setting Tracking Tool Mode = " << trackMultTools << std::endl;
    }
    
    // add pre-defined tools (up to 100)
    for (unsigned int i = 0; i < 100; i++) {
        std::stringstream context;
        std::string stealthName, name;
        context << "/tracker/tools/tool[" << i+1 << "]";
        config.GetXMLValue(context.str().c_str(), "@stealthName", stealthName, "");
        config.GetXMLValue(context.str().c_str(), "@name", name, "");
        if (stealthName.empty() && name.empty()) {
            break;
        }
        if (stealthName.empty()) {
            AddTool(name, name);
        } else if (name.empty()) {
            AddTool(stealthName, stealthName);
        } else {
            AddTool(stealthName, name);
        }
    }

#ifdef sawMedtronicStealthlink_IS_SIMULATOR
    CMN_LOG_CLASS_INIT_VERBOSE << "Configure: using simulated Stealthstation" << std::endl;
    StealthlinkPresent = true;
#else
    CMN_LOG_CLASS_INIT_VERBOSE << "Configure: initializing Stealthlink" << std::endl;
    StealthlinkPresent = this->Client->Initialize(*(this->Utils)) ? true : false;
    if (!StealthlinkPresent) {
        CMN_LOG_CLASS_RUN_WARNING << "Configure: could not Initialize StealthLink" << std::endl;
    }
#endif
}


void mtsMedtronicStealthlink::ResetAllTools(void)
{
    ToolsContainer::iterator it;
    const ToolsContainer::iterator end = Tools.end();
    for (it = Tools.begin(); it != end; it++) {
        (*it)->MarkerPosition.SetValid(false);
        (*it)->TooltipPosition.SetValid(false);
    }
}


mtsMedtronicStealthlink::Tool * mtsMedtronicStealthlink::FindTool(const std::string & stealthName) const
{
    ToolsContainer::const_iterator it;
    const ToolsContainer::const_iterator end = Tools.end();
    for (it = Tools.begin(); it != end; it++) {
        if ((*it)->GetStealthName() == stealthName) {
            return *it;
        }
    }
    return 0;
}


mtsMedtronicStealthlink::Tool * mtsMedtronicStealthlink::AddTool(const std::string & stealthName, const std::string & interfaceName)
{
    // First, check if tool has already been added
    Tool * tool = FindTool(stealthName);
    if (tool) {
        if (tool->GetInterfaceName() == interfaceName) {
            CMN_LOG_CLASS_RUN_WARNING << "AddTool: tool " << stealthName << " already exists with interface "
                                      << interfaceName << std::endl;
            return tool;
        }
        // We could support having the same tool in multiple interfaces, but we would need to maintain
        // an array of CurrentTools, or loop through the entire Tools list in the Run method (to assign the
        // MarkerPosition and TooltipPosition).
        CMN_LOG_CLASS_RUN_ERROR << "AddTool: tool " << stealthName << " already exists in interface "
                                << tool->GetInterfaceName() << ", could not create new interface "
                                << interfaceName << std::endl;
        return 0;
    }

    // Next, check if interface has already been added
    mtsInterfaceProvided * provided = GetInterfaceProvided(interfaceName);
    if (provided) {
        CMN_LOG_CLASS_RUN_ERROR << "AddTool: interface " << interfaceName << " already exists." << std::endl;
        return 0;
    }

    // Create the tool and add it to the list
    tool = new Tool(stealthName, interfaceName);
    Tools.push_back(tool);
    CMN_LOG_CLASS_RUN_VERBOSE << "AddTool: adding " << stealthName << " to interface " << interfaceName << std::endl;
    provided = AddInterfaceProvided(interfaceName);
    if (provided) {
        StateTable.AddData(tool->TooltipPosition, interfaceName + "Position");
        provided->AddCommandReadState(StateTable, tool->TooltipPosition, "GetPositionCartesian");
        StateTable.AddData(tool->MarkerPosition, interfaceName + "Marker");
        provided->AddCommandReadState(StateTable, tool->MarkerPosition, "GetMarkerCartesian");
    }
    return tool;
}


void mtsMedtronicStealthlink::RequestExamInformation(void)
{
    if (StealthlinkPresent) {
        exam_info the_exam_info;
#ifndef sawMedtronicStealthlink_IS_SIMULATOR
        this->Client->GetDataForCode(GET_EXAM_INFO,
                                     reinterpret_cast<void*>(&the_exam_info));
#endif
        ExamInformationMember.VoxelScale[0] = the_exam_info.voxel_scale[0];
        ExamInformationMember.VoxelScale[1] = the_exam_info.voxel_scale[1];
        ExamInformationMember.VoxelScale[2] = the_exam_info.voxel_scale[2];
        ExamInformationMember.Size[0] = the_exam_info.size[0];
        ExamInformationMember.Size[1] = the_exam_info.size[1];
        ExamInformationMember.Size[2] = the_exam_info.size[2];
        ExamInformationMember.Valid = the_exam_info.valid;
    }
}


void mtsMedtronicStealthlink::RequestSurgicalPlan(void)
{
    if (StealthlinkPresent) {
        surg_plan the_surg_plan;
#ifndef sawMedtronicStealthlink_IS_SIMULATOR
        this->Client->GetDataForCode(GET_SURGICAL_PLAN,
                                     reinterpret_cast<void*>(&the_surg_plan));
#endif
        SurgicalPlan[0] = the_surg_plan.entry[0];
        SurgicalPlan[1] = the_surg_plan.entry[1];
        SurgicalPlan[2] = the_surg_plan.entry[2];
        SurgicalPlan[3] = the_surg_plan.target[0];
        SurgicalPlan[4] = the_surg_plan.target[1];
        SurgicalPlan[5] = the_surg_plan.target[2];
    }
}


void mtsMedtronicStealthlink::GetSurgicalPlan(mtsDoubleVec & plan) const
{
    plan = SurgicalPlan;
}


void mtsMedtronicStealthlink::Run(void)
{
    ResetAllTools();  // Set all tools invalid

    if (StealthlinkPresent) {
#ifndef sawMedtronicStealthlink_IS_SIMULATOR

        all_info info;
        multtool mult_tool;
        probe_calibration probe_cal;
        mult_probe_calibration mult_probe_cal;

        tool * mtools[MAX_MULT_TOOLS];
        probe_calibration * mcals[MAX_MULT_TOOLS];

        unsigned int iteration_num;

        // Get the data from Stealthlink.
        if (TrackMultTools) {   // MultTools mode
            
            this->Client->GetDataForCode(GET_MULT_TOOL, reinterpret_cast<void*>(&mult_tool));

            mtools[0] = &mult_tool.Tool0;
            mtools[1] = &mult_tool.Tool1;
            mtools[2] = &mult_tool.Tool2;
            mtools[3] = &mult_tool.Tool3;
            mtools[4] = &mult_tool.Tool4;
            
            mcals[0] = &mult_probe_cal.ProbeCal0;
            mcals[1] = &mult_probe_cal.ProbeCal1;
            mcals[2] = &mult_probe_cal.ProbeCal2;
            mcals[3] = &mult_probe_cal.ProbeCal3;
            mcals[4] = &mult_probe_cal.ProbeCal4;
            
            frame frame_data;
            this->Client->GetDataForCode(GET_FRAME, reinterpret_cast<void*>(&frame_data));
            FrameData = frame_data;

            registration registration_data;
            this->Client->GetDataForCode(GET_REGISTRATION, reinterpret_cast<void*>(&registration_data));
            RegistrationData = registration_data;

            iteration_num = MAX_MULT_TOOLS;

        } else {    // SingleTool mode
            
            this->Client->GetDataForCode(GET_ALL, reinterpret_cast<void*>(&info));
            
            // set data for "controller" interface (note: this uses non
            // standard cisst types and should probably be removed later (adeguet1)

            mtools[0] = &info.Tool;
            mcals[0] = &probe_cal;
            
            FrameData = info.Frame;
            RegistrationData = info.Reg;
            
            iteration_num = 1;
        }

#else
        // Compute some simulated data
        const ToolsContainer::const_iterator firstTool = Tools.begin();
        if (firstTool != Tools.end()) {
            tool simulatedTool;
            for (unsigned int i = 0; i < 3; i++) {
                for (unsigned int j = 0; j < 3; j++) {
                    if (i == j) {
                        simulatedTool.xform[i][j] = 1.0;
                    } else {
                        simulatedTool.xform[i][j] = 0.0;
                    }
                }
                simulatedTool.xform[i][3] = static_cast<float>(i);
            }
            simulatedTool.geometry_error = 1.11;
            const std::string toolName = (*firstTool)->GetStealthName();
            for (unsigned int k = 0; k < NAME_LENGTH; k++) {
                if (k < toolName.size()) {
                    simulatedTool.name[k] = toolName[k];
                } else {
                    simulatedTool.name[k] = '\0';
                }
            }
            simulatedTool.valid = true;
            ToolData = &simulatedTool;
        }
#endif            
        // update Multiple Tool data
        for (unsigned int i = 0; i < iteration_num; i++) {
            if (mtools[i]->valid) {

                MultToolData = *mtools[i];

                if (!CurrentMultTool[i] || (CurrentMultTool[i]->GetStealthName() != MultToolData.GetName())) {
                    CurrentMultTool[i] = FindTool(MultToolData.GetName());
                    // PK: I don't think we should call AddTool in the Run method. If the tool was not defined in
                    // the XML file, we should ignore it.
#if 0
                    if (!CurrentMultTool[i]) {
                        CMN_LOG_CLASS_INIT_VERBOSE << "Run: adding new tool \""
                                                   << MultToolData.GetName() << "\"" << std::endl;
                        CurrentMultTool[i] = AddTool(MultToolData.GetName(), MultToolData.GetName());
                    }
                    if (CurrentMultTool[i]) {
                        CMN_LOG_CLASS_RUN_VERBOSE << "Run: current tool is now \""
                                                  << CurrentMultTool[i]->GetInterfaceName() << "\"" << std::endl;
                    } else {
                        CMN_LOG_CLASS_RUN_ERROR << "Run: unable to add provided interface for new tool \""
                                                << MultToolData.GetName() << "\"" << std::endl;
                    }
#else
                    if (CurrentMultTool[i])
                        CMN_LOG_CLASS_RUN_VERBOSE << "Run: current tool " << i << " is now \""
                                                  << CurrentMultTool[i]->GetInterfaceName() << "\"" << std::endl;
                    else
                        CMN_LOG_CLASS_INIT_VERBOSE << "Run: ignoring unconfigured tool \""
                                                   << MultToolData.GetName() << "\"" << std::endl;
#endif
                }
                // rely on older interface to retrieve tool information
                if (CurrentMultTool[i]) {
                    CurrentMultTool[i]->MarkerPosition.Position() = MultToolData.GetFrame();
                    CurrentMultTool[i]->MarkerPosition.SetValid(true);
                }

                ProbeCal = *mcals[i];

                // Get tool tip calibration if it is invalid or has changed
                if ((strcmp(MultToolData.GetName(), ProbeCal.GetName()) != 0) || !ProbeCal.Valid()) {
#ifndef sawMedtronicStealthlink_IS_SIMULATOR
                    
                    if (TrackMultTools)
                        this->Client->GetDataForCode(GET_MULT_PROBE_CALIBRATION, reinterpret_cast<void*>(&mult_probe_cal));
                    else
                        this->Client->GetDataForCode(GET_PROBE_CALIBRATION, reinterpret_cast<void*>(&probe_cal));
                    
                    ProbeCal = *mcals[i];
#endif
                }else
                {
                    std::cout << "did not get got probe cal " << ToolData.GetName() << std::endl;
                }


                // If we have valid data, then store the result
                if (CurrentMultTool[i] && ProbeCal.Valid() &&
                    (strcmp(MultToolData.GetName(), ProbeCal.GetName()) == 0)) {
                    CurrentMultTool[i]->TooltipPosition.Position() = vctFrm3( MultToolData.GetFrame().Rotation(),
                                                                              MultToolData.GetFrame() * ProbeCal.GetTip());
                    CurrentMultTool[i]->TooltipPosition.SetValid(true);
                    std::cout << "ProbeCal[" << i << "]: " << ProbeCal.GetTip() << std::endl;
                } else {
                    if(!CurrentMultTool[i])
                        std::cout << "CurrentMultTool[" << i << "] not valid" << std::endl;
                    if(!ProbeCal.Valid())
                        std::cout << "ProbeCal not valid" << ProbeCal.Valid() << std::endl;
                    if(!(strcmp(MultToolData.GetName(), ProbeCal.GetName()) == 0))
                        std::cout << MultToolData.GetName() << " does not match " << ProbeCal.GetName() << std::endl;
                }
            }
        }

        // update frame interface data
        if (FrameData.Valid()) {
            if (!CurrentFrame || (CurrentFrame->GetStealthName() != FrameData.GetName())) {
                CurrentFrame = FindTool(FrameData.GetName());
                if (!CurrentFrame) {
                    CMN_LOG_CLASS_INIT_VERBOSE << "Run: adding new tool \""
                                               << FrameData.GetName() << "\"" << std::endl;
                    CurrentFrame = AddTool(FrameData.GetName(), FrameData.GetName());
                }
                if (CurrentFrame) {
                    CMN_LOG_CLASS_RUN_VERBOSE << "Run: current tool is now \""
                                              << CurrentFrame->GetInterfaceName() << "\"" << std::endl;
                } else {
                    CMN_LOG_CLASS_RUN_ERROR << "Run: unable to add provided interface for new tool \""
                                            << FrameData.GetName() << "\"" << std::endl;
                }
            }
            // rely on older interface to retrieve tool information
            if (CurrentFrame) {
                CurrentFrame->MarkerPosition.Position() = FrameData.GetFrame();
                CurrentFrame->MarkerPosition.SetValid(true);
            }
        }

        // update registration interface data
        this->RegistrationMember.Transformation = RegistrationData.GetFrame();
        this->RegistrationMember.Valid = RegistrationData.Valid();
        this->RegistrationMember.PredictedAccuracy = RegistrationData.GetAccuracy();
        this->RegistrationMember.PredictedAccuracy.SetValid(RegistrationData.Valid());
    }
    ProcessQueuedCommands();
    this->Utils->CheckCallbacks(); //not in SL2?
}


void mtsMedtronicStealthlink::Cleanup(void)
{
    ToolsContainer::iterator it;
    const ToolsContainer::iterator end = Tools.end();
    for (it = Tools.begin(); it != end; it++) {
        delete (*it);
        *it = 0;
    }
    Tools.clear();
    if (this->Client) {
        delete this->Client;
        this->Client = 0;
    }
    if (this->Utils) {
        delete this->Utils;
        this->Utils = 0;
    }
    CMN_LOG_CLASS_RUN_VERBOSE << "Cleanup: finished" << std::endl;
}
