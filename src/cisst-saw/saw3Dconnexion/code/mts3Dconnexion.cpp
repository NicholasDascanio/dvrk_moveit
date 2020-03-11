/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*

  Author(s):  Marcin Balicki
  Created on: 2008-04-12

  (C) Copyright 2008-2012 Johns Hopkins University (JHU), All Rights
  Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---
*/

#include <cisstConfig.h>
#include <cisstVector/vctDynamicVectorTypes.h>
#include <cisstMultiTask/mtsInterfaceProvided.h>
#include <saw3Dconnexion/mts3Dconnexion.h>
#include <saw3DconnexionConfig.h>

#if (CISST_OS == CISST_WINDOWS)
#include <Windows.h>
#import "progid:TDxInput.Device.1" no_namespace
#elif (CISST_OS == CISST_DARWIN)
#include <3DconnexionClient/ConnexionClientAPI.h>
#elif (SAW_HAS_SPACENAV)
//see http://spacenav.sourceforge.net/faq.html
#include <spnav.h>
#endif

CMN_IMPLEMENT_SERVICES_DERIVED_ONEARG(mts3Dconnexion, mtsTaskPeriodic, mtsTaskPeriodicConstructorArg);

class mts3DconnexionData
{
  public:
#if (CISST_OS == CISST_WINDOWS)
    ISimpleDevicePtr _3DxDevice;
    ISensor * m_p3DSensor;
    IKeyboard * m_p3DKeyboard;
    ISimpleDevicePtr pSimpleDevice;
    MSG Msg;
    IVector3DPtr trans;
    IAngleAxisPtr rot;
#elif (CISST_OS == CISST_DARWIN)
    UInt16 ClientID;
    bool Button1Pressed;
    bool Button2Pressed;
    bool Button3Pressed;
#endif

#if (SAW_HAS_SPACENAV)
    spnav_event SpnavEvent;
#endif
};


void mts3DconnexionInternalMessageHandler(mts3Dconnexion * instance, const vctDynamicVector<double> & axis, const vctDynamicVector<bool> & buttons)
{
    instance->DataTable->Start();
    instance->Axis.Assign(axis);
    instance->Buttons.Assign(buttons);
    instance->UpdateDataTable();
    instance->DataTable->Advance();
}


#if (CISST_OS == CISST_DARWIN)
// global map to retrieve instance from clientID in message handler
typedef std::map<UInt16, mts3Dconnexion *> saw3DconnexionIdToInstanceMapType;
saw3DconnexionIdToInstanceMapType saw3DconnexionIdToInstanceMap;

// message handler for Cocoa events on Mac
void mts3DconnexionMessageHandler(io_connect_t, natural_t messageType, void * messageArgument)
{
    ConnexionDeviceState * state;
    saw3DconnexionIdToInstanceMapType::iterator instance;
    switch (messageType) {
      case kConnexionMsgDeviceState:
        state = reinterpret_cast<ConnexionDeviceState *>(messageArgument);
        instance = saw3DconnexionIdToInstanceMap.find(state->client);
        if (instance != saw3DconnexionIdToInstanceMap.end()) {
            vctDoubleVec axis(6);
            for (unsigned int i = 0; i < axis.size(); ++i) {
                axis[i] = state->axis[i];
            }
            vctBoolVec buttons(2);
            buttons[0] = (state->buttons == 1) | (state->buttons == 3);
            buttons[1] = (state->buttons == 2) | (state->buttons == 3);
            mts3DconnexionInternalMessageHandler(instance->second, axis, buttons);
        }
        break;
      default:
        CMN_LOG_RUN_DEBUG << "Received unhandled kConnexionCmd type: " << messageType << std::endl;
        break;
    }
}
#endif


void mts3Dconnexion::Cleanup(void)
{
#if (CISST_OS == CISST_WINDOWS)
    if (Data->m_p3DSensor) {
        Data->m_p3DSensor->get_Device((IDispatch**)&(Data->_3DxDevice));
        Data->m_p3DSensor->Release();
    }
    if (Data->m_p3DKeyboard) {
        Data->m_p3DKeyboard->Release();
    }
    if (Data->_3DxDevice) {
        Data->_3DxDevice->Disconnect();
        Data->_3DxDevice->Release();
    }
#endif

#if (SAW_HAS_SPACENAV)
    spnav_close();
#endif

}


void mts3Dconnexion::Configure(const std::string & configurationName)
{
    Data = new mts3DconnexionData;
    ConfigurationName = configurationName;
    Axis.SetSize(6);
    Axis.SetAll(0.0);
    Buttons.SetSize(2);
    Buttons.SetAll(false);
    Mask.SetSize(6);
    Mask.SetAll(true);
    Gain = 1.0;

    DataTable = new mtsStateTable(StateTable.GetHistoryLength(), "3Dconnexion");
    AddStateTable(DataTable);
#if (CISST_OS == CISST_DARWIN || SAW_HAS_SPACENAV)
    DataTable->SetAutomaticAdvance(false);  // state table is populated in MessageHandler
#else
    DataTable->SetAutomaticAdvance(true);  // state table is populated in Run
#endif
    DataTable->AddData(Axis, "AxisData");
    DataTable->AddData(Buttons, "ButtonData");
    DataTable->AddData(Mask, "AxisMask");
    DataTable->AddData(Gain, "Gain");
    DataTable->AddData(Position, "Position");
    DataTable->AddData(IsConnected, "IsConnected");

    mtsInterfaceProvided * providesSpaceNavigator = AddInterfaceProvided("ProvidesSpaceNavigator");
    if (providesSpaceNavigator) {
        providesSpaceNavigator->AddCommandReadState(*DataTable, Axis, "GetAxisData");
        providesSpaceNavigator->AddCommandReadState(*DataTable, Buttons, "GetButtonData");
        providesSpaceNavigator->AddCommandReadState(*DataTable, Mask, "GetAxisMask");
        providesSpaceNavigator->AddCommandWriteState(*DataTable, Mask, "SetAxisMask");
        providesSpaceNavigator->AddCommandReadState(*DataTable, Gain, "GetGain");
        providesSpaceNavigator->AddCommandWriteState(*DataTable, Gain, "SetGain");
        providesSpaceNavigator->AddCommandReadState(*DataTable, Position, "GetPositionCartesian");
        providesSpaceNavigator->AddCommandVoid(&mts3Dconnexion::ReBias, this, "ReBias");
        providesSpaceNavigator->AddCommandReadState(*DataTable, IsConnected, "GetIsConnected");
    }

#if (CISST_OS == CISST_DARWIN)
    OSErr result = InstallConnexionHandlers(mts3DconnexionMessageHandler, 0L, 0L);
    if (result != noErr) {
        CMN_LOG_CLASS_INIT_ERROR << "Failed do install handlers (error " << result << ")" << std::endl;
        IsConnected = false;
        return;
    }
    std::string clientName = "3DxSAW";
    size_t l = clientName.size();
    unsigned char * pascalString = new unsigned char[l+2];
    pascalString[0] = static_cast<unsigned char>(l);
    pascalString[l+1] = 0;
    size_t i = 0;
    while (i < l) {
        pascalString[i + 1] = static_cast<unsigned char>(clientName[i]);
        ++i;
    }
    this->Data->ClientID = RegisterConnexionClient(kConnexionClientWildcard, pascalString,
                                                   kConnexionClientModeTakeOver,
                                                   kConnexionMaskAll);
    delete [] pascalString;
    saw3DconnexionIdToInstanceMap[this->Data->ClientID] = this;  // save pointer to allow callbacks to modify this object
    CMN_LOG_CLASS_INIT_VERBOSE << "SpaceNavigator is registered with ID: " << this->Data->ClientID << std::endl;
#endif

#if (SAW_HAS_SPACENAV)
    if(spnav_open()==-1) {
        CMN_LOG_CLASS_INIT_ERROR << ("failed to connect to the space navigator daemon\n ");
        IsConnected = false;
        return;
    }
#endif
    IsConnected = true;
}


void mts3Dconnexion::Startup(void)
{
#if (CISST_OS == CISST_WINDOWS)
    HRESULT hr = ::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (!SUCCEEDED(hr)) {
        CMN_LOG_CLASS_INIT_ERROR << "Failed to CoInitializeEx" << std::endl;
        IsConnected = false;
        DataTable->Advance();
        return;
    }
    hr = Data->_3DxDevice.CreateInstance(__uuidof(Device));
    if (!SUCCEEDED(hr)) {
        CMN_LOG_CLASS_INIT_ERROR << "Failed to CreateInstance" << std::endl;
        IsConnected = false;
        DataTable->Advance();
        return;
    }
    Data->pSimpleDevice = Data->_3DxDevice;
    long type;
    hr = Data->_3DxDevice->QueryInterface(&(Data->pSimpleDevice));
    if (!SUCCEEDED(hr)) {
        CMN_LOG_CLASS_INIT_ERROR << "Failed to QueryInterface" << std::endl;
        IsConnected = false;
        DataTable->Advance();
        return;
    }
    hr = Data->pSimpleDevice->get_Type(&type);
    if (!SUCCEEDED(hr)) {
        CMN_LOG_CLASS_INIT_ERROR << "Failed to get_Type" << std::endl;
        IsConnected = false;
        DataTable->Advance();
        return;
    }
    Data->m_p3DSensor = Data->pSimpleDevice->Sensor;
    Data->m_p3DKeyboard = Data->pSimpleDevice->Keyboard;
    Data->pSimpleDevice->LoadPreferences(ConfigurationName.c_str());
    hr = Data->pSimpleDevice->Connect();  // this returns no matter if the device is connected or not
    if (!SUCCEEDED(hr)) {
        CMN_LOG_CLASS_INIT_ERROR << "Failed to Connect" << std::endl;
        IsConnected = false;
        DataTable->Advance();
        return;
    }
    CMN_LOG_CLASS_INIT_VERBOSE << "SpaceNavigator is initialized" << std::endl;
#endif

    IsConnected = true;
    DataTable->Advance();
}


void mts3Dconnexion::Run(void)
{
    ProcessQueuedCommands();

#if (CISST_OS == CISST_WINDOWS)
    if (PeekMessage(&(Data->Msg), NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&(Data->Msg));
        DispatchMessage(&(Data->Msg));
    }
    if (Data->m_p3DSensor) {
        try {
            double angle;
            Data->trans = Data->m_p3DSensor->GetTranslation();
            Data->rot = Data->m_p3DSensor->GetRotation();
            Data->rot->get_Angle(&angle);
            Axis[0] = Data->trans->GetX();
            Axis[1] = Data->trans->GetY();
            Axis[2] = Data->trans->GetZ();
            Axis[3] = Data->rot->GetX() * angle;
            Axis[4] = Data->rot->GetY() * angle;
            Axis[5] = Data->rot->GetZ() * angle;
        } catch (...) {
            CMN_LOG_CLASS_RUN_ERROR << "Caught exception" << std::endl;
        }
    }
    if (Data->m_p3DKeyboard) {
        try {
            for (unsigned int i = 0; i < Buttons.size(); ++i) {
                Buttons[i] = Data->m_p3DKeyboard->IsKeyDown(i+1) == VARIANT_TRUE;
            }
        } catch (...) {
            CMN_LOG_CLASS_RUN_ERROR << "Caught exception" << std::endl;
        }
    }
    UpdateDataTable();
#endif

#if (SAW_HAS_SPACENAV)
    //clean out all the samples in the state table.
    while (spnav_poll_event(&Data->SpnavEvent) != 0) {
        DataTable->Start();
        if(Data->SpnavEvent.type == SPNAV_EVENT_MOTION) {
            //left handed coordinate system - fix it:
            //X to the right (as looking at the sign)
            //Y is down
            //Z is back (towards cable)
            Axis[0] = Data->SpnavEvent.motion.x;
            Axis[1] = -Data->SpnavEvent.motion.y;
            Axis[2] = Data->SpnavEvent.motion.z;
            Axis[3] = Data->SpnavEvent.motion.rx;
            Axis[4] = -Data->SpnavEvent.motion.ry;
            Axis[5] = Data->SpnavEvent.motion.rz;

            // the limits are +/- 350 for all inputs

        }
        else {	/* SPNAV_EVENT_BUTTON */
            Buttons[Data->SpnavEvent.button.bnum] = Data->SpnavEvent.button.press;
        }
        UpdateDataTable();
        DataTable->Advance();
    }
#endif
}

void mts3Dconnexion::UpdateDataTable(void)
{
    // apply mask and gain to axis data
    for (unsigned int i = 0; i < Axis.size(); ++i) {
        if (Mask[i]) {
            Axis[i] *= Gain;
        } else {
            Axis[i] = 0.0;
        }
    }

    // accumulate applied force to provide an absolute Cartesian position
    for (unsigned int i = 0; i < 3; ++i) {
        Translation[i] += Axis[i];
        Orientation[i] += Axis[i+3];
    }
    Position.Position().Translation().Assign(Translation);
    Position.Position().Rotation().From(vctEulerZYXRotation3(Orientation));
}
