/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*
  $Id: mtsMedtronicStealthlinkTypes.cpp 3596 2012-04-11 20:08:50Z dmirota1 $
  Author(s): Peter Kazanzides, Anton Deguet
  Created on: 2007

  (C) Copyright 2007-2011 Johns Hopkins University (JHU), All Rights Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---
*/

#include <sawMedtronicStealthlink/mtsMedtronicStealthlink2Types.h>

// for conversion method
#include <sawMedtronicStealthlink/mtsMedtronicStealthlink2.h>

// Stealthlink definitions
#ifdef CISST_HAS_STEALTHLINK
#include <StealthLink/StealthLink.h>
#endif

CMN_IMPLEMENT_SERVICES(mtsStealthTool);
CMN_IMPLEMENT_SERVICES(mtsStealthFrame);
CMN_IMPLEMENT_SERVICES(mtsStealthRegistration);
CMN_IMPLEMENT_SERVICES(mtsStealthProbeCal);


typedef float floatArray44[4][4];

void frameConversion(vctFrm3 & result, const floatArray44 & input) {
    size_t row, col;
    for (row = 0; row < 3; row++) {
        for (col = 0; col < 3; col++) {
            result.Rotation().at(row, col) =  input[row][col];
        }
        result.Translation().at(row) = input[row][3];
    }
}

void mtsStealthTool::Assign(const mtsStealthTool & that)
{
    this->SetTimestamp(that.Timestamp());
    this->XForm = that.XForm;
    this->GeometryError = that.GeometryError;
    for (int k = 0; k < NAME_LENGTH; k++) {
        this->Name[k] = that.Name[k];
    }
    this->Valid() = that.Valid();
}

void mtsStealthTool::Assign(const MNavStealthLink::Instrument & griTool)
{
    frameConversion(this->XForm, griTool.localizer_T_instrument);
    GeometryError = griTool.geometryError;
    for (int k = 0; k < NAME_LENGTH; k++) {
        Name[k] = griTool.name[k];
    }
    this->SetValid(griTool.visibility == MNavStealthLink::Instrument::VISIBLE || griTool.visibility == MNavStealthLink::Instrument::ALMOST_BLOCKED);
}

void mtsStealthTool::Assign(const prmPositionCartesianGet & that)
{
    this->SetTimestamp(that.Timestamp());
    this->XForm = that.Position();
    this->GeometryError = 0;
    for (int k = 0; k < NAME_LENGTH; k++) {
        this->Name[k] = 'n';
    }
    this->Name[NAME_LENGTH-1] = '\0';
    this->Valid() = that.Valid();
}

std::string mtsStealthTool::ToString(void) const
{
    std::stringstream outputStream;
    ToStream(outputStream);
    return outputStream.str();
}

void mtsStealthTool::ToStream(std::ostream & outputStream) const
{
    outputStream << Name << ", " << XForm << ", " << GeometryError << std::endl;
}

void mtsStealthTool::ToStreamRaw(std::ostream & outputStream, const char delimiter,
                                 bool headerOnly, const std::string & headerPrefix) const
{
    if (headerOnly) {
        mtsGenericObject::ToStreamRaw(outputStream, delimiter, headerOnly, headerPrefix);
        outputStream << delimiter << headerPrefix << "-Name";
        outputStream << delimiter;
        XForm.ToStreamRaw(outputStream, delimiter, headerOnly, headerPrefix);
        outputStream << delimiter << headerPrefix << "-GeomError";
    }
    else {
        mtsGenericObject::ToStreamRaw(outputStream, delimiter);
        outputStream << delimiter;
        outputStream << Name << delimiter;
        XForm.ToStreamRaw(outputStream, delimiter);
        outputStream << delimiter;
        outputStream << GeometryError;
    }
}

void mtsStealthTool::SerializeRaw(std::ostream & outputStream) const
{
    mtsGenericObject::SerializeRaw(outputStream);
    std::string name = this->Name;
    cmnSerializeRaw(outputStream, name);
    this->XForm.SerializeRaw(outputStream);
    cmnSerializeRaw(outputStream, this->GeometryError);
}

void mtsStealthTool::DeSerializeRaw(std::istream & inputStream)
{
    mtsGenericObject::DeSerializeRaw(inputStream);
    std::string name;
    cmnDeSerializeRaw(inputStream, name);
    strncpy(this->Name, name.c_str(), name.size() + 1); // copy all characters include \0
    this->XForm.DeSerializeRaw(inputStream);
    cmnDeSerializeRaw(inputStream, this->GeometryError);
}


myGenericObject * mtsStealthTool::operator= (const MNavStealthLink::DataItem & item_in)
{
    this->Assign(dynamic_cast<const MNavStealthLink::Instrument &>(item_in));
    if(my_parent){
        mtsMedtronicStealthlink::Tool * CurrentTool = my_parent->FindTool(this->Name);
        my_parent->ProbeCal.Assign(dynamic_cast<const MNavStealthLink::Instrument &>(item_in));

        if (this->Valid()) {
            /*if (!CurrentTool || (CurrentTool->GetStealthName() != this->GetName())) {
                CurrentTool = FindTool(ToolData.GetName());
                if (!CurrentTool) {
                    CMN_LOG_CLASS_INIT_VERBOSE << "Run: adding new tool \""
                                               << ToolData.GetName() << "\"" << std::endl;
                    CurrentTool = AddTool(ToolData.GetName(), ToolData.GetName());
                }
                if (CurrentTool) {
                    CMN_LOG_CLASS_RUN_VERBOSE << "Run: current tool is now \""
                                              << CurrentTool->GetInterfaceName() << "\"" << std::endl;
                } else {
                    CMN_LOG_CLASS_RUN_ERROR << "Run: unable to add provided interface for new tool \""
                                            << ToolData.GetName() << "\"" << std::endl;
                }
            }*/
            // rely on older interface to retrieve tool information
            if (CurrentTool) {
                CurrentTool->MarkerPosition.Position() = this->GetFrame();
                CurrentTool->MarkerPosition.SetValid(true);
            /*}
            // Get tool tip calibration if it is invalid or has changed
            if ((strcmp(ToolData.GetName(), ProbeCal.GetName()) != 0) || !ProbeCal.Valid()) {


                std::cout << "Got probe cal " << ToolData.GetName() << std::endl;
                std::cout << "   probe cal " << ProbeCal.GetName() << std::endl;
                std::cout << "   tip " << ProbeCal.GetTip() << std::endl;
                std::cout << "   hind " << ProbeCal.GetHind() << std::endl;
                std::cout << "   valid " << ProbeCal.Valid() << std::endl;

            }else
            {
                std::cout << "did not get got probe cal " << ToolData.GetName() << std::endl;
            }
            // If we have valid data, then store the result
            if (CurrentTool && ProbeCal.Valid() &&
                (strcmp(ToolData.GetName(), ProbeCal.GetName()) == 0)) {*/
                CurrentTool->TooltipPosition.Position() = vctFrm3(this->GetFrame().Rotation(),
                                                                  this->GetFrame() * my_parent->ProbeCal.GetTip());
                CurrentTool->TooltipPosition.SetValid(true);
            }else
            {
                if(!CurrentTool)
                    std::cout << "CurrentTool not valid" << my_parent->ProbeCal.Valid() << std::endl;
                if(!my_parent->ProbeCal.Valid())
                    std::cout << "ProbeCal not valid" << my_parent->ProbeCal.Valid() << std::endl;
                if(!(strcmp(this->GetName(), my_parent->ProbeCal.GetName()) == 0))
                    std::cout << this->GetName() << " does not match " << my_parent->ProbeCal.GetName() << std::endl;

            }
        }

    }
    return this;
}



void mtsStealthFrame::Assign(const mtsStealthFrame & that)
{
    this->SetTimestamp(that.Timestamp());
    this->XForm = that.XForm;
    this->GeometryError = that.GeometryError;
    for (int k = 0; k < NAME_LENGTH; k++) this->Name[k] = that.Name[k];
    this->Valid() = that.Valid();
}

void mtsStealthFrame::Assign (const MNavStealthLink::Frame & griFrame)
{
    frameConversion(this->XForm, griFrame.frame_T_localizer);
    GeometryError = griFrame.geometryError;
    for (int k = 0; k < NAME_LENGTH; k++) Name[k] = griFrame.name[k];
    this->SetValid(griFrame.visibility == MNavStealthLink::Frame::VISIBLE || griFrame.visibility == MNavStealthLink::Instrument::ALMOST_BLOCKED);
}

void mtsStealthFrame::Assign(const prmPositionCartesianGet & that)
{
    this->SetTimestamp(that.Timestamp());
    this->XForm = that.Position();
    this->GeometryError = 0;
    for (int k = 0; k < NAME_LENGTH; k++) this->Name[k] = 'n';
    this->Name[NAME_LENGTH-1] = '\0';
    this->Valid() = that.Valid();
}

std::string mtsStealthFrame::ToString(void) const
{
    std::stringstream outputStream;
    ToStream(outputStream);
    return outputStream.str();
}

void mtsStealthFrame::ToStream(std::ostream & outputStream) const
{
    outputStream << Name << ", " << XForm << ", " << GeometryError << std::endl;
}

void mtsStealthFrame::ToStreamRaw(std::ostream & outputStream, const char delimiter,
                                  bool headerOnly, const std::string & headerPrefix) const
{
    if (headerOnly) {
        mtsGenericObject::ToStreamRaw(outputStream, delimiter, headerOnly, headerPrefix);
        outputStream << delimiter << headerPrefix << "-Name";
        outputStream << delimiter;
        XForm.ToStreamRaw(outputStream, delimiter, headerOnly, headerPrefix);
        outputStream << delimiter << headerPrefix << "-GeomError";
    }
    else {
        mtsGenericObject::ToStreamRaw(outputStream, delimiter);
        outputStream << delimiter;
        outputStream << Name << delimiter;
        XForm.ToStreamRaw(outputStream, delimiter);
        outputStream << delimiter << GeometryError;
    }
}


myGenericObject * mtsStealthFrame::operator= (const MNavStealthLink::DataItem & item_in)
{
    this->Assign(dynamic_cast<const MNavStealthLink::Frame &>(item_in));
    if(my_parent){
        mtsMedtronicStealthlink::Tool * CurrentFrame = my_parent->FindTool(this->Name);

        if (this->Valid()) {
            /*if (!CurrentFrame || (CurrentFrame->GetStealthName() != this->GetName())) {
                CurrentFrame = FindTool(FrameData.GetName());
                if (!CurrentFrame) {
                    //CMN_LOG_CLASS_INIT_VERBOSE << "Run: adding new tool \""
                    //                           << FrameData.GetName() << "\"" << std::endl;
                    //CurrentFrame = AddTool(FrameData.GetName(), FrameData.GetName());
                }
                if (CurrentFrame) {
                    //CMN_LOG_CLASS_RUN_VERBOSE << "Run: current tool is now \""
                    //                          << CurrentFrame->GetInterfaceName() << "\"" << std::endl;
                } else {
                    //CMN_LOG_CLASS_RUN_ERROR << "Run: unable to add provided interface for new tool \""
                    //                        << FrameData.GetName() << "\"" << std::endl;
                }
            }*/
            // rely on older interface to retrieve tool information
            if (CurrentFrame) {
                CurrentFrame->MarkerPosition.Position() = this->GetFrame();
                CurrentFrame->MarkerPosition.SetValid(true);
            }
        }

    }
    return this;
}


void mtsStealthRegistration::Assign(const mtsStealthRegistration & that)
{
    this->SetTimestamp(that.Timestamp());
    this->XForm = that.XForm;
    this->predictedAccuracy = that.predictedAccuracy;
    this->Valid() = that.Valid();
}

void mtsStealthRegistration::Assign (const MNavStealthLink::Registration & griRegistration)
{
    frameConversion(this->XForm, griRegistration.regExamMM_T_frame);
    predictedAccuracy = griRegistration.predictedAccuracy;
    this->SetValid(true);
}

void mtsStealthRegistration::Assign(const vctFrm3 & tmpFrm, const double & tmpAccuracy,
                                    const bool & tmpValid)
{
    XForm = tmpFrm;
    predictedAccuracy = tmpAccuracy;
    this->SetValid(tmpValid);
}

std::string mtsStealthRegistration::ToString(void) const {
    std::stringstream outputStream;
    ToStream(outputStream);
    return outputStream.str();
}

void mtsStealthRegistration::ToStream(std::ostream & outputStream) const
{
    outputStream << XForm << ", " << predictedAccuracy << std::endl;
}

void mtsStealthRegistration::ToStreamRaw(std::ostream & outputStream, const char delimiter,
                                         bool headerOnly, const std::string & headerPrefix) const
{
    if (headerOnly) {
        mtsGenericObject::ToStreamRaw(outputStream, delimiter, headerOnly, headerPrefix);
        outputStream << delimiter;
        XForm.ToStreamRaw(outputStream, delimiter, headerOnly, headerPrefix);
        outputStream << delimiter << headerPrefix << "-PredAcc";
    }
    else {
        mtsGenericObject::ToStreamRaw(outputStream, delimiter);
        outputStream << delimiter;
        XForm.ToStreamRaw(outputStream, delimiter);
        outputStream << delimiter << predictedAccuracy;
    }
}

myGenericObject * mtsStealthRegistration::operator= (const MNavStealthLink::DataItem & item_in)
{
    this->Assign(dynamic_cast<const MNavStealthLink::Registration &>(item_in));
    if (my_parent){
        my_parent->RegistrationMember.Transformation = this->GetFrame();
        my_parent->RegistrationMember.Valid = this->Valid();
        my_parent->RegistrationMember.PredictedAccuracy = this->GetAccuracy();
        my_parent->RegistrationMember.PredictedAccuracy.SetValid(this->Valid());
    }
    return this;
}


void mtsStealthProbeCal::Assign(const mtsStealthProbeCal & that)
{
    this->SetTimestamp(that.Timestamp());
    for (int k = 0; k < NAME_LENGTH; k++) this->Name[k] = that.Name[k];
    this->Valid() = that.Valid();
    this->Tip = that.Tip;
    this->Hind = that.Hind;
}

void mtsStealthProbeCal::Assign (const MNavStealthLink::Instrument & griProbeCal)
{
    for (int k = 0; k < NAME_LENGTH; k++) Name[k] = griProbeCal.name[k];
    this->SetValid(true);
    //const_cast required because MNavStealthLink::Point operator[] is not const
    MNavStealthLink::InstrumentPosition & current_ProbeCal = const_cast<MNavStealthLink::InstrumentPosition &>(griProbeCal.localizerPosition);
    Tip = vct3(current_ProbeCal.tip[0],current_ProbeCal.tip[1],current_ProbeCal.tip[2]);
    Hind = vct3(current_ProbeCal.hind[0],current_ProbeCal.hind[1],current_ProbeCal.hind[2]);
}

std::string mtsStealthProbeCal::ToString(void) const
{
    std::stringstream outputStream;
    ToStream(outputStream);
    return outputStream.str();
}

void mtsStealthProbeCal::ToStream(std::ostream & outputStream) const
{
    outputStream << Name << ", " << Valid() << ", " << Tip << ", " << Hind << std::endl;
}

void mtsStealthProbeCal::ToStreamRaw(std::ostream & outputStream, const char delimiter,
                                     bool headerOnly, const std::string & headerPrefix) const
{
    if (headerOnly) {
        mtsGenericObject::ToStreamRaw(outputStream, delimiter, headerOnly, headerPrefix);
        outputStream << delimiter << headerPrefix << "-Name";
        Tip.ToStreamRaw(outputStream, delimiter, headerOnly, headerPrefix + "Tip");
        outputStream << delimiter;
        Hind.ToStreamRaw(outputStream, delimiter, headerOnly, headerPrefix + "Hind");
    }
    else {
        mtsGenericObject::ToStreamRaw(outputStream, delimiter);
        outputStream << delimiter;
        outputStream << Name << delimiter;
        Tip.ToStreamRaw(outputStream, delimiter);
        outputStream << delimiter;
        Hind.ToStreamRaw(outputStream, delimiter);
    }
}

myGenericObject * mtsStealthProbeCal::operator= (const MNavStealthLink::DataItem & item_in)
{
    this->Assign(dynamic_cast<const MNavStealthLink::Instrument &>(item_in));
    return this;
}

