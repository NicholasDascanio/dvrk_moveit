/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*

 Author(s):  Ankur Kapoor, Tian Xia
 Created on: 2004-04-30

 (C) Copyright 2004-2011 Johns Hopkins University (JHU), All Rights
 Reserved.

 --- begin cisst license - do not edit ---

 This software is provided "as is" under an open source license, with
 no warranty.  The complete license can be found in license.txt and
 http://www.cisst.org/cisst/license.txt.

 --- end cisst license ---
 */

#include <sawLoPoMoCo/mtsLoPoMoCo.h>
#include <cisstCommon/cmnXMLPath.h>
#include <cisstMultiTask/mtsInterfaceProvided.h>

#include "osaLoPoMoCoBoardIO.h"
#include "osaLoPoMoCoOffsets.h"
#include <limits>

CMN_IMPLEMENT_SERVICES(mtsLoPoMoCo);

mtsLoPoMoCo::mtsLoPoMoCo(const std::string & deviceName, unsigned int numberOfBoards) :
    mtsComponent(deviceName)
{
    this->numberOfBoards = numberOfBoards;
    numberOfAxes = NB_AXIS * numberOfBoards;

    CMN_LOG_CLASS_INIT_VERBOSE <<" Number of axes " << numberOfAxes<<std::endl;

    // assigning sizes to variables
    BaseAddress.resize(numberOfBoards);
    Board.resize(numberOfBoards);
    StartAxis.resize(numberOfBoards);
    EndAxis.resize(numberOfBoards);
    MaxAxis.resize(numberOfBoards);

    MotorVoltages.SetSize(numberOfAxes);
    EncoderFrequencies.SetSize(numberOfAxes);
    FrequencyToRPSRatio.SetSize(numberOfAxes);
    CountsToMotorCurrents.SetSize(numberOfAxes);
    CountsToPotFeedback.SetSize(numberOfAxes);
    MotorSpeedToCounts.SetSize(numberOfAxes);
    PositiveSlope.SetSize(numberOfAxes);
    PositiveIntercept.SetSize(numberOfAxes);
    NegativeSlope.SetSize(numberOfAxes);
    NegativeIntercept.SetSize(numberOfAxes);
    VoltageToCounts.SetSize(numberOfAxes);

    mtsInterfaceProvided * provided = AddInterfaceProvided("WriteInterface");
    if(provided) {
        //void commands, no parameters
        provided->AddCommandVoid(&mtsLoPoMoCo::LatchEncoders, this, "LatchEncoders");
        provided->AddCommandVoid(&mtsLoPoMoCo::StartMotorCurrentConv, this, "StartMotorCurrentConv");
        provided->AddCommandVoid(&mtsLoPoMoCo::StartPotFeedbackConv, this, "StartPotFeedbackConv");
        provided->AddCommandVoid(&mtsLoPoMoCo::StartPotFeedbackConvFast, this, "StartPotFeedbackConvFast");
        provided->AddCommandVoid(&mtsLoPoMoCo::LoadMotorVoltages, this, "LoadMotorVoltages");
        provided->AddCommandVoid(&mtsLoPoMoCo::LoadCurrentLimits, this, "LoadCurrentLimits");
        provided->AddCommandVoid(&mtsLoPoMoCo::LoadMotorVoltagesCurrentLimits, this, "LoadMotorVoltagesCurrentLimits");
        provided->AddCommandVoid(&mtsLoPoMoCo::EnableAll, this, "EnableAll");
        provided->AddCommandVoid(&mtsLoPoMoCo::DisableAll, this, "DisableAll");

        // read commands
        provided->AddCommandRead(&mtsLoPoMoCo::GetPositions, this, "GetPositions", mtsLongVec(numberOfAxes));
        provided->AddCommandRead(&mtsLoPoMoCo::GetVelocities, this, "GetVelocities", mtsShortVec(numberOfAxes));
        provided->AddCommandRead(&mtsLoPoMoCo::GetMotorCurrents, this, "GetMotorCurrents",mtsShortVec(numberOfAxes));
        provided->AddCommandRead(&mtsLoPoMoCo::GetPotFeedbacks, this, "GetPotFeedbacks",mtsShortVec(numberOfAxes));
        provided->AddCommandRead(&mtsLoPoMoCo::GetDigitalInput, this, "GetDigitalInput",mtsIntVec(numberOfAxes));
        // GSF -- GetLatchedIndex might only be available with 0xCCDD FPGA (MR-Robot)
        provided->AddCommandRead(&mtsLoPoMoCo::GetLatchedIndex, this, "GetLatchedIndex",mtsShortVec(numberOfAxes));

        // Write methods
        // method , object carrying the method , interface name , command name and argument prototype
        provided->AddCommandWrite(&mtsLoPoMoCo::SetMotorVoltages, this, "SetMotorVoltages", mtsShortVec(numberOfAxes));
        provided->AddCommandWrite(&mtsLoPoMoCo::SetCurrentLimits, this, "SetCurrentLimits", mtsShortVec(numberOfAxes));
        provided->AddCommandWrite(&mtsLoPoMoCo::SetDigitalOutput, this, "SetDigitalOutput", mtsIntVec(numberOfBoards));

        provided->AddCommandWrite(&mtsLoPoMoCo::Enable, this, "Enable", mtsShort());
        provided->AddCommandWrite(&mtsLoPoMoCo::Disable, this, "Disable", mtsShort());
        provided->AddCommandWrite(&mtsLoPoMoCo::ResetEncoders, this, "ResetEncoders", mtsShort());
        provided->AddCommandWrite(&mtsLoPoMoCo::SetPositions, this, "SetPositions", mtsLongVec(numberOfAxes));

        // Qualified Read
        // method , object carrying the method , interface name , command name and argument prototype A and argument prototype B
        provided->AddCommandQualifiedRead(&mtsLoPoMoCo::FrequencyToRPS, this,
                                          "FrequencyToRPS", mtsShortVec(numberOfAxes), mtsDoubleVec(numberOfAxes));
        provided->AddCommandQualifiedRead(&mtsLoPoMoCo::ADCToMotorCurrents, this,
                                          "ADCToMotorCurrents",mtsShortVec(numberOfAxes), mtsDoubleVec(numberOfAxes));
        provided->AddCommandQualifiedRead(&mtsLoPoMoCo::ADCToPotFeedbacks, this,
                                          "ADCToPotFeedbacks",mtsShortVec(numberOfAxes), mtsDoubleVec(numberOfAxes));
        provided->AddCommandQualifiedRead(&mtsLoPoMoCo::MotorVoltagesToDAC, this,
                                          "MotorVoltagesToDAC", mtsDoubleVec(numberOfAxes),mtsShortVec(numberOfAxes));
        provided->AddCommandQualifiedRead(&mtsLoPoMoCo::CurrentLimitsToDAC, this,
                                          "CurrentLimitsToDAC", mtsDoubleVec(numberOfAxes),mtsShortVec(numberOfAxes));
    }
    // set the relative path to configuration files
    //RelativePathToConfigFiles(relativePathToConfigFiles);

}

mtsLoPoMoCo::~mtsLoPoMoCo() {
    for (int boardIndex = 0; boardIndex < numberOfBoards; boardIndex++) {
        Board[boardIndex]->DisableAllMotors();
        if (Board[boardIndex])
            delete Board[boardIndex];
    }
}

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

void mtsLoPoMoCo::parseInputArgument(const std::string &inputArgument, std::string &relativeFilePath, std::string &fileName)
{
    unsigned int posSeparator = inputArgument.find('/', 0);
    while (posSeparator != inputArgument.npos) {
        relativeFilePath = inputArgument.substr(0, posSeparator);
        fileName = inputArgument.substr(posSeparator+1, inputArgument.size());
        posSeparator = inputArgument.find('/', posSeparator+1); // find the next '/'
    }

    relativeFilePath += "/"; // append a '/' at the end
    CMN_LOG_CLASS_INIT_VERBOSE << " input file name: "<<inputArgument<<","<<"relative file path: "<<relativeFilePath<<","
                               << " file name: "<<fileName<<std::endl;

}

// TODO: Tian: make sure that boardIndex counts starts at 0 AND axis count starts at 0
void mtsLoPoMoCo::ConfigureOneBoard(const std::string& filename, const int boardIndex) {

    // Example@ daVinciLeftMasterBoard1.xml, high level xml: daVinciLoPoMoCoMasterLeft.xml
    if (filename == "") {
        CMN_LOG_CLASS_INIT_ERROR << "ConfigureOneBoard: could not configure single LoPoMoCo device" << std::endl;
        return;
    }

    struct stat st;
    if (stat(filename.c_str(), &st) < 0) {
        CMN_LOG_CLASS_INIT_ERROR << "ConfigureOneBoard: Invalid filename!! " << filename << std::endl;
        return;
    }

    CMN_LOG_CLASS_INIT_VERBOSE << "Configuring a LoPoMoCo board with \"" << filename <<"\"" << std::endl;
    cmnXMLPath xmlConfig;
    xmlConfig.SetInputSource(filename);
    char path[60];
    //std::string context("/Config/Device[@Name=\"");
    //xmlConfig.GetXMLValue(context.c_str(), "@Name",
    //context = context + GetName() + "\"]";
    std::string context("/Config/Device");

    //read in the base address and axis factors
    bool ret = xmlConfig.GetXMLValue(context.c_str(), "@BaseAddress", BaseAddress[boardIndex]);

    int SelectorSwitchPosition;
    if (ret == false) {
        // try switch selector
        xmlConfig.GetXMLValue(context.c_str(), "@SelectorSwitchPosition", SelectorSwitchPosition);
        BaseAddress[boardIndex] = 0x200 + SelectorSwitchPosition * 0x20;
    }
    xmlConfig.GetXMLValue(context.c_str(), "@StartAxis", StartAxis[boardIndex]);
    xmlConfig.GetXMLValue(context.c_str(), "@EndAxis", EndAxis[boardIndex]);

    CMN_LOG_CLASS_INIT_DEBUG << "StartAxis: " << StartAxis[boardIndex] << "   EndAxis: " << EndAxis[boardIndex] << std::endl << "BaseAddress: " << BaseAddress[boardIndex] << std::endl;

    // MaxAxis is the number of axes used on a board
    MaxAxis[boardIndex] = EndAxis[boardIndex] - StartAxis[boardIndex];

    //Board = new ddiLoPoMoCoLowLevelIO(BaseAddress);
    Board[boardIndex] = new osaLoPoMoCoBoardIO(BaseAddress[boardIndex]);

    int version = Board[boardIndex]->BoardVersion();
    // the latest version return 0xb0b || 0xS0 where S = ~(switch value)
    // version 1 boards return 0xa0a
    // some Board[boardIndex] with 10K50E also return 0xc0c
    // The MR-Robot FPGA (which emulates a LoPoMoCo) returns 0xCCDD
    CMN_LOG_CLASS_INIT_VERBOSE << "Version: " << version << std::endl;
    if (!( ( version & 0xFF0F )== 0xb0b || (version == 0xa0a) || (version == 0xc0c) || (version == 0xCCDD) )) {
        CMN_LOG_CLASS_INIT_VERBOSE << "WARNING: Could not find a LoPoMoCo board at address (decimal) " << BaseAddress[boardIndex] << std::endl;
        CMN_LOG_CLASS_INIT_VERBOSE << "Actually, this just means that the version number does not match" <<std::endl;
    }

    //unsigned short listEncoders[] = {1, 1, 1, 1};
    Board[boardIndex]->SetADInterruptNumber(0x05);
    Board[boardIndex]->SetTimerInterruptNumber(0x05);
    Board[boardIndex]->SetADInterrupt(false);
    Board[boardIndex]->SetTimerInterrupt(false);
    Board[boardIndex]->SetWDTPeriod(0x2FF);
    //Board->SetEncoderPreloadRegister(0x007FFFFF);
    //Board->PreLoadEncoders(listEncoders);
    //Board->DisableAllMotors();
    if (version == 0xCCDD) {
        // Start with all valves/motors disabled (off)
        Board[boardIndex]->DisableAllMotors();
    }
    else
        Board[boardIndex]->EnableAllMotors();

    CMN_LOG_CLASS_INIT_VERBOSE << "Board initialization" <<std::endl;

    int axis;
    //read in the conversion factors
    for (unsigned int count = 0; count < 4; count++) {
        // compute the corrected axis index for the composite device from multiple boards
        axis = StartAxis[boardIndex] + count;
        CMN_LOG_CLASS_INIT_VERBOSE << "Configuring axis"<<axis<<std::endl;

        sprintf(path, "Axis[%d]/Encoder/@FrequencyToRPS", count+1);
        xmlConfig.GetXMLValue(context.c_str(), path, FrequencyToRPSRatio[axis]);
        // anton sprintf(path, "Axis[%d]/Encoder/@ShaftRevToEncoder", axis+1);
        // anton xmlConfig.GetXMLValue(context.c_str(), path, CountsToDeg[axis]);

        sprintf(path, "Axis[%d]/ADC/@CountsToMotorCurrents", count+1);
        xmlConfig.GetXMLValue(context.c_str(), path, CountsToMotorCurrents[axis]);
        sprintf(path, "Axis[%d]/ADC/@CountsToPotFeedback", count+1);
        xmlConfig.GetXMLValue(context.c_str(), path, CountsToPotFeedback[axis]);

        sprintf(path, "Axis[%d]/DAC/@MotorSpeedToCounts", count+1);
        xmlConfig.GetXMLValue(context.c_str(), path, MotorSpeedToCounts[axis]);
        sprintf(path, "Axis[%d]/DAC/CurrentLimitToCounts/@PositiveSlope", count+1);
        xmlConfig.GetXMLValue(context.c_str(), path, PositiveSlope[axis]);
        sprintf(path, "Axis[%d]/DAC/CurrentLimitToCounts/@PositiveIntercept", count+1);
        xmlConfig.GetXMLValue(context.c_str(), path, PositiveIntercept[axis]);
        sprintf(path, "Axis[%d]/DAC/CurrentLimitToCounts/@NegativeSlope", count+1);
        xmlConfig.GetXMLValue(context.c_str(), path, NegativeSlope[axis]);
        sprintf(path, "Axis[%d]/DAC/CurrentLimitToCounts/@NegativeIntercept", count+1);
        xmlConfig.GetXMLValue(context.c_str(), path, NegativeIntercept[axis]);
        sprintf(path, "Axis[%d]/DAC/CurrentLimitToCounts/@VoltageToCounts", count+1);
        xmlConfig.GetXMLValue(context.c_str(), path, VoltageToCounts[axis]);
    }
    CMN_LOG_CLASS_INIT_VERBOSE << "Configured a LoPoMoCo board: #" <<boardIndex<< std::endl;
}

/*!
 * TODO: Tian: Top level XML file
 */
void mtsLoPoMoCo::Configure(const std::string& filename){ //, const std::string & relativePathToConfigFiles) {

    //justFileName is the top-level file name, and is NOT the full path file name
    std::string relativePathToConfigFiles, justFileName;

    // filename has the file name with the full path to the file
    // this function separates filename into the path and the actual filename
    // we need the path in order to read other files embedded in the top-level configuration file
    parseInputArgument(filename, relativePathToConfigFiles, justFileName);

    CMN_LOG_CLASS_INIT_VERBOSE << "Configure: configuring a LoPoMoCo device with " << numberOfBoards
                               << " boards, and " << numberOfAxes << " axis total" <<std::endl;

    if (justFileName == "") {
        CMN_LOG_CLASS_INIT_ERROR << "Configure: could not configure LoPoMoCo device" << std::endl;
        return;
    }

    // add relative path to file
    justFileName.insert(0, relativePathToConfigFiles);

    struct stat st;
    if (stat(justFileName.c_str(), &st) < 0) {
        CMN_LOG_CLASS_INIT_ERROR << "Configure: invalid justFileName!! " << justFileName << std::endl;
        return;
    }


    CMN_LOG_CLASS_INIT_VERBOSE << "Configure: configuring LoPoMoCo with \"" << justFileName << "\"" << std::endl;
    cmnXMLPath xmlConfig;
    xmlConfig.SetInputSource(justFileName);

    char path[60];
    std::string context("/Config/Device[@Name=\"");
    context = context + GetName() + "\"]";

    std::vector<std::string> configFiles(numberOfBoards);
    int boardIndex;
    bool allFilesDefined = true;

    // get names of files first
    for (boardIndex = 0; boardIndex < numberOfBoards; boardIndex++) {
        sprintf(path, "ConfigFile[%d]/@Name", boardIndex + 1);
        allFilesDefined &= xmlConfig.GetXMLValue(context.c_str(), path, configFiles[boardIndex]);
    }

    // Backward compatibility: if there is just one board and we failed to find a config file
    // entry in the passed XML file, then use the passed XML file as the config file.
    if (!allFilesDefined && (numberOfBoards == 1)) {
        parseInputArgument(filename, relativePathToConfigFiles, configFiles[0]);
        allFilesDefined = true;
    }

    if(allFilesDefined) {
        for (boardIndex = 0; boardIndex < numberOfBoards; boardIndex++) {
            //configFiles[boardIndex].insert(0, RelativePathToConfigFiles);
            configFiles[boardIndex].insert(0, relativePathToConfigFiles);
            CMN_LOG_CLASS_INIT_VERBOSE << "Configure: configuring board " << boardIndex + 1 << " with " << configFiles[boardIndex] << std::endl;
            ConfigureOneBoard(configFiles[boardIndex].c_str(), boardIndex);
        }
    } else {
        CMN_LOG_CLASS_INIT_ERROR << "Configure: couldn't find all the required ConfigFile in " << justFileName << std::endl;
    }
}

/*! axisIndex is a flat index into the composite device with multiple boards,
 *  this axisIndex is mapped onto the correct board index and axis index within that board
 *  action taken on "Board" is always with respect to a 4 axes board
 */
void mtsLoPoMoCo::Enable(const mtsShort & axisIndex) {
    int boardIndex;
    boardIndex = MapAxisToBoard(axisIndex.Data);
    if (boardIndex != -1) {
        CMN_LOG_CLASS_RUN_VERBOSE << "Enabling motor " << axisIndex.Data << std::endl;
        Board[boardIndex]->EnableMotor(axisIndex.Data - StartAxis[boardIndex]);
    } else {
        CMN_LOG_CLASS_RUN_ERROR << "Enabling motor " << axisIndex.Data
                                << " failed since the index is out of range ["
                                << StartAxis[boardIndex]<< ", " << EndAxis[boardIndex] << "]" << std::endl;
    }
}
void mtsLoPoMoCo::Disable(const mtsShort & axisIndex) {
    int boardIndex;
    boardIndex = MapAxisToBoard(axisIndex.Data);
    if (boardIndex != -1) {
        CMN_LOG_CLASS_RUN_VERBOSE << "Disabling motor " << axisIndex.Data << std::endl;
        Board[boardIndex]->DisableMotor(axisIndex.Data - StartAxis[boardIndex]);
    } else {
        CMN_LOG_CLASS_RUN_ERROR << "Disabling motor " << axisIndex.Data
                                << " failed since the index is out of range ["
                                << StartAxis[boardIndex]<< ", " << EndAxis[boardIndex] << "]" << std::endl;
    }
}

void mtsLoPoMoCo::ResetEncoders(const mtsShort & axisIndex) {
    unsigned short listEncoders[] = { 0, 0, 0, 0 };
    int boardIndex = MapAxisToBoard(axisIndex.Data);

    if (boardIndex != -1) {
        CMN_LOG_CLASS_RUN_VERBOSE << "Resetting encoder " << axisIndex.Data << std::endl;
        Board[boardIndex]->SetEncoderIndices(false, 0x00, axisIndex.Data-StartAxis[boardIndex]);
        listEncoders[axisIndex.Data-StartAxis[boardIndex]] = 1;
        Board[boardIndex]->SetEncoderPreloadRegister(0x007FFFFF);
        Board[boardIndex]->PreLoadEncoders(listEncoders);
        //Board->SetEncoderIndices(true, MaxAxis, 0x00);
    } else {
        CMN_LOG_CLASS_RUN_ERROR << "Resetting encoder " << axisIndex.Data
                                << " failed since the index is out of range ["
                                << StartAxis[boardIndex] << ", " << EndAxis[boardIndex] << "]" << std::endl;
    }
}

void mtsLoPoMoCo::EnableAll(void) {
    for (int boardIndex = 0; boardIndex < numberOfBoards; boardIndex++) {
        Board[boardIndex]->EnableAllMotors();
    }
}

void mtsLoPoMoCo::DisableAll(void) {
    for (int boardIndex = 0; boardIndex < numberOfBoards; boardIndex++) {
        Board[boardIndex]->DisableAllMotors();
    }
}

void mtsLoPoMoCo::LatchEncoders(void) {
    for (int boardIndex = 0; boardIndex < numberOfBoards; boardIndex++) {
        Board[boardIndex]->LatchEncoders();
    }
}

void mtsLoPoMoCo::StartMotorCurrentConv(void) {
    for (int boardIndex = 0; boardIndex < numberOfBoards; boardIndex++) {
        Board[boardIndex]->StartConvCurrentFeedback();
    }
}

void mtsLoPoMoCo::StartPotFeedbackConv(void) {
    for (int boardIndex = 0; boardIndex < numberOfBoards; boardIndex++) {
        Board[boardIndex]->StartConvPotFeedback();
    }
}

void mtsLoPoMoCo::StartPotFeedbackConvFast(void) {
    for (int boardIndex = 0; boardIndex < numberOfBoards; boardIndex++) {
        Board[boardIndex]->StartConvPotFeedbackFast();
    }
}

// TODO: Tian: Need review!
// Question: WHAT DOES MaxAxis mean, and what does SetEncoderIndices do?
// for now the velocities need to be read before the encoder positions
// are read. This is because the FPGA has the same index register for
// all three.  So we cache the period and frequency, which is against
// the principals of not caching data in the hardware device
// driver. Hum, should we rethink this, or let this be an exception
// and allow such exception with the goodwill assumption that they
// would be documented where needed.
void mtsLoPoMoCo::GetPositions(mtsLongVec & Positions) const {
    for (int boardIndex = 0; boardIndex < numberOfBoards; boardIndex++) {
        //Board[boardIndex]->SetEncoderIndices(true, MaxAxis[boardIndex], 0x00);
        for (unsigned int axis = 0; axis <= MaxAxis[boardIndex]; axis++) {
            //EncoderPeriods.Data[axis] = Board->GetEncoderPeriod();
            //EncoderFrequencies.Data[axis] = Board->GetEncoderFrequency();
            Board[boardIndex]->SetEncoderIndices(false, MaxAxis[boardIndex], axis);
            Positions[axis + StartAxis[boardIndex]] = Board[boardIndex]->GetEncoder() - 0x007FFFFF;
            //CMN_LOG_CLASS_INIT_ERROR <<"position" <<Positions<<std::endl;
        }
    }
}

void mtsLoPoMoCo::GetVelocities(mtsShortVec& Velocities) const {
    for (int boardIndex = 0; boardIndex < numberOfBoards; boardIndex++) {
        for (unsigned int axis = 0; axis <= MaxAxis[boardIndex]; axis++) {
            Board[boardIndex]->SetEncoderIndices(false, MaxAxis[boardIndex], axis);
            Velocities[axis + StartAxis[boardIndex]] = Board[boardIndex]->GetEncoderFrequency();
        }
    }
}

// Precondition:  StartConvCurrentFeedback should be called before calling GetPotFeedback
void mtsLoPoMoCo::GetMotorCurrents(mtsShortVec & MotorCurrents) const {
    bool ADInterruptPending = false;
    for (int boardIndex = 0; boardIndex < numberOfBoards; boardIndex++) {
        Board[boardIndex]->SetCurrentFeedbackMaxIndex(MaxAxis[boardIndex]);
        ADInterruptPending = Board[boardIndex]->PollADInterruptPending(20); //5 * MaxAxis[boardIndex]);
        if (ADInterruptPending == true) {
            for (unsigned int axis = 0; axis <= MaxAxis[boardIndex]; axis++) {
                MotorCurrents[axis + StartAxis[boardIndex]] = Board[boardIndex]->GetADFIFO();
            }
        } else {
            for (unsigned int axis = 0; axis <= MaxAxis[boardIndex]; axis++)
                MotorCurrents[axis + StartAxis[boardIndex]] = 0x8000;
        }
    }
}

// Precondition:  StartConvPotFeedback should be called before calling GetPotFeedback
void mtsLoPoMoCo::GetPotFeedbacks(mtsShortVec & PotFeedbacks) const {
    bool ADInterruptPending = false;
    for (int boardIndex = 0; boardIndex < numberOfBoards; boardIndex++) {
        Board[boardIndex]->SetPotFeedbackMaxIndex(MaxAxis[boardIndex]);
        ADInterruptPending = Board[boardIndex]->PollADInterruptPending(20); //5 * MaxAxis[boardIndex]);
        if (ADInterruptPending == true) {
            for (unsigned int axis = 0; axis <= MaxAxis[boardIndex]; axis++) {
                PotFeedbacks[axis + StartAxis[boardIndex]] = Board[boardIndex]->GetADFIFO();
            }
        } else {
            for (unsigned int axis = 0; axis <= MaxAxis[boardIndex]; axis++)
                PotFeedbacks[axis + StartAxis[boardIndex]] = 0x8000;
        }
    }
}

void mtsLoPoMoCo::GetDigitalInput(mtsIntVec & DigitalInput) const {
    for (int boardIndex = 0; boardIndex < numberOfBoards; boardIndex++) {
        DigitalInput[StartAxis[boardIndex]/4] = Board[boardIndex]->GetDigitalInput();
    }
}

void mtsLoPoMoCo::SetMotorVoltages(const mtsShortVec & _MotorVoltages) {
    //cached the motor volatages because they are required for converting
    //current limits to dac counts
    for (int boardIndex = 0; boardIndex < numberOfBoards; boardIndex++) {
        for (unsigned int axis = 0; axis <= MaxAxis[boardIndex]; axis++) {
            Board[boardIndex]->SetMotorVoltageIndices(false, MaxAxis[boardIndex], axis);
            Board[boardIndex]->SetMotorVoltage(_MotorVoltages[axis + StartAxis[boardIndex]]);
            //this->MotorVoltages[axis + StartAxis[boardIndex]] = MotorVoltages[axis + StartAxis[boardIndex]];
            this->MotorVoltages[axis] = _MotorVoltages[axis + StartAxis[boardIndex]];
            //if (MotorVoltages.Data[axis] >= 4090) MotorVoltages.Data[axis] = 4090;
            //if (MotorVoltages.Data[axis] <= -4090) MotorVoltages.Data[axis] = -4090;
        }
    }
}

void mtsLoPoMoCo::SetCurrentLimits(const mtsShortVec & CurrentLimits) {
    for (int boardIndex = 0; boardIndex < numberOfBoards; boardIndex++) {
        for (unsigned int axis = 0; axis <= MaxAxis[boardIndex]; axis++) {
            Board[boardIndex]->SetCurrentLimitIndices(false, MaxAxis[boardIndex], axis);
            Board[boardIndex]->SetCurrentLimit(CurrentLimits[axis + StartAxis[boardIndex]]);
        }
    }
}

void mtsLoPoMoCo::SetPositions(const mtsLongVec & Positions){
    for (int boardIndex = 0; boardIndex < numberOfBoards; boardIndex++) {
        // use auto increment and set one by one
        unsigned short listEncoders[] = {0, 0, 0, 0};
        for (unsigned int axis = 0; axis <= MaxAxis[boardIndex]; axis++) {
            Board[boardIndex]->SetEncoderIndices(false, MaxAxis[boardIndex], axis);
            listEncoders[axis] = 1;
            Board[boardIndex]->SetEncoderPreloadRegister(Positions[StartAxis[boardIndex] + axis] + 0x007FFFFF);
            Board[boardIndex]->PreLoadEncoders(listEncoders);
            listEncoders[axis] = 0;
        }
    }
}

void mtsLoPoMoCo::SetDigitalOutput(const mtsIntVec & DigitalOutput){
    for (int boardIndex = 0; boardIndex < numberOfBoards; boardIndex++) {
        Board[boardIndex]->SetDigitalOutput(DigitalOutput[StartAxis[boardIndex]/4]);
    }
}

void mtsLoPoMoCo::LoadMotorVoltages(void){
    for (int boardIndex = 0; boardIndex < numberOfBoards; boardIndex++) {
        Board[boardIndex]->LoadMotorVoltages();
    }
}

void mtsLoPoMoCo::LoadCurrentLimits(void){
    for (int boardIndex = 0; boardIndex < numberOfBoards; boardIndex++) {
        Board[boardIndex]->LoadCurrentLimits();
    }
}

void mtsLoPoMoCo::LoadMotorVoltagesCurrentLimits(void){
    for (int boardIndex = 0; boardIndex < numberOfBoards; boardIndex++) {
        Board[boardIndex]->LoadMotorVoltagesCurrentLimits();
    }
}

void mtsLoPoMoCo::FrequencyToRPS(const mtsShortVec& fromData, mtsDoubleVec & toData) const {
    for (int boardIndex = 0; boardIndex < numberOfBoards; boardIndex++) {
        for (unsigned int axis = 0; axis <= MaxAxis[boardIndex]; axis++) {
            toData[axis + StartAxis[boardIndex]] = FrequencyToRPSRatio[StartAxis[boardIndex] + axis]
                / ((double) (fromData[axis + StartAxis[boardIndex]]));
        }
    }
}

void mtsLoPoMoCo::ADCToMotorCurrents(const mtsShortVec& fromData, mtsDoubleVec & toData) const {
    for (int boardIndex = 0; boardIndex < numberOfBoards; boardIndex++) {
        for (unsigned int axis = 0; axis <= MaxAxis[boardIndex]; axis++) {
            toData[axis + StartAxis[boardIndex]] = CountsToMotorCurrents[StartAxis[boardIndex] + axis]
                * ((double) (fromData[axis + StartAxis[boardIndex]]));
        }
    }
}

void mtsLoPoMoCo::ADCToPotFeedbacks(const mtsShortVec& fromData, mtsDoubleVec & toData) const {
    for (int boardIndex = 0; boardIndex < numberOfBoards; boardIndex++) {
        for (unsigned int axis = 0; axis <= MaxAxis[boardIndex]; axis++) {
            toData[axis + StartAxis[boardIndex]] = CountsToPotFeedback[StartAxis[boardIndex] + axis]
                * ((double) (fromData[axis + StartAxis[boardIndex]]));
        }
    }
}

void mtsLoPoMoCo::MotorVoltagesToDAC(const mtsDoubleVec& fromData, mtsShortVec& toData) const {
    double dacCounts;
    for (int boardIndex = 0; boardIndex < numberOfBoards; boardIndex++) {
        for (unsigned int axis = 0; axis <= MaxAxis[boardIndex]; axis++) {
            dacCounts = MotorSpeedToCounts[axis+StartAxis[boardIndex]]*fromData[axis+StartAxis[boardIndex]];
            if (dacCounts >= 4090) dacCounts = 4090;
            if (dacCounts <= -4090) dacCounts = -4090;
            toData[axis+StartAxis[boardIndex]] = (short)(dacCounts);
        }
    }
}

void mtsLoPoMoCo::CurrentLimitsToDAC(const mtsDoubleVec& fromData, mtsShortVec& toData) const {
    double dac2InVolts;
    for (int boardIndex = 0; boardIndex < numberOfBoards; boardIndex++) {
        for (unsigned int axis = 0; axis <= MaxAxis[boardIndex]; axis++) {
            if (this->MotorVoltages[axis + StartAxis[boardIndex]] >=0 ) {
                dac2InVolts = PositiveSlope[axis + StartAxis[boardIndex]]*fromData[axis+StartAxis[boardIndex]] + PositiveIntercept[axis+ StartAxis[boardIndex]];
            } else {
                dac2InVolts = NegativeSlope[axis + StartAxis[boardIndex]]*fromData[axis+StartAxis[boardIndex]] + NegativeIntercept[axis+ StartAxis[boardIndex]];
            }
            toData[axis+StartAxis[boardIndex]] = (short)(VoltageToCounts[axis+ StartAxis[boardIndex]]*dac2InVolts);
        }
    }
}

// GSF - added 11/13/07 to get latched index values
// Might only be available in Version 0xCCDD FPGA (MR-Robot)
void mtsLoPoMoCo::GetLatchedIndex(mtsShortVec & latchedIndex) const
{
    for (int boardIndex = 0; boardIndex < numberOfBoards; boardIndex++) {
        for (unsigned int axis = 0; axis <= MaxAxis[boardIndex]; axis++) {
            Board[boardIndex]->SetEncoderIndices(false, MaxAxis[boardIndex], axis);
            latchedIndex[axis+StartAxis[boardIndex]] = Board[boardIndex]->GetEncoderIndex() - 0x007FFFFF;
        }
    }
}
