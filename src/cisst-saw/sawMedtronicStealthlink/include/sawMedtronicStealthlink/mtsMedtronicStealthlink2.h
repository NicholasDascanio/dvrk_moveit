/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*
  $Id: mtsMedtronicStealthlink.h 3754 2012-07-27 15:06:53Z dmirota1 $

  Author(s): Peter Kazanzides, Anton Deguet, Daniel Mirota
  Created on: 2006

  (C) Copyright 2006-2011 Johns Hopkins University (JHU), All Rights Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---
*/

#ifndef _mtsMedtronicStealthlink_h
#define _mtsMedtronicStealthlink_h

#include <cisstMultiTask/mtsTaskFromSignal.h>
#include <cisstMultiTask/mtsTransformationTypes.h>
#include <cisstMultiTask/mtsFixedSizeVectorTypes.h>
#include <cisstParameterTypes/prmPositionCartesianGet.h>

// Always include last
#include <sawMedtronicStealthlink/sawMedtronicStealthlinkExport.h>

// forward declarations of Stealthlink types
namespace MNavStealthLink {
    class StealthServer;
    class Exam;
    class SurgicalPlan;
    class Instrument;
    class Frame;
    class Registration;
    class InstrumentPosition;
    class DataItem;
    template <typename T> class Subscription;
}

class myGenericObject{
    protected:
        std::string myName;
        mtsStateTable myStateTable;
        myGenericObject(const std::string & name):myName(name), myStateTable(256,myName + "Table"){
            myStateTable.SetAutomaticAdvance(false);
        }
    public:
        void AssignAndAdvance(const MNavStealthLink::DataItem & item_in){
            this->Assign(item_in);
            myStateTable.Advance();
        }
        virtual void Assign(const MNavStealthLink::DataItem & item_in) = 0;

        virtual void ConfigureInterfaceProvided(mtsInterfaceProvided * provided_in) = 0;
};



class CISST_EXPORT mtsMedtronicStealthlink: public mtsTaskFromSignal
{
    CMN_DECLARE_SERVICES(CMN_DYNAMIC_CREATION_ONEARG, CMN_LOG_ALLOW_DEFAULT);


    MNavStealthLink::StealthServer * StealthServerProxy;

    osaThread StealthServerProxyThread;

    void * StealthlinkRun(void *);

    bool StealthlinkPresent;

    // Class used to store tool information using cisstParameterTypes
    class Tool: public myGenericObject
    {
        std::string StealthName;
        std::string InterfaceName;
    public:
        Tool(const std::string &stealthName, const std::string &interfaceName): myGenericObject(interfaceName),
            StealthName(stealthName), InterfaceName(interfaceName) {this->myStateTable.AddData(this->TooltipPosition,this->myName + "Position");
                                                                                                 this->myStateTable.AddData(this->MarkerPosition,this->myName + "Marker");}
        ~Tool(void) {}
        const std::string & GetStealthName(void) const { return StealthName; }
        const std::string & GetInterfaceName(void) const { return InterfaceName; }

        void Assign(const MNavStealthLink::DataItem & item_in);
        void ConfigureInterfaceProvided(mtsInterfaceProvided * provided_in);

        prmPositionCartesianGet TooltipPosition;
        prmPositionCartesianGet MarkerPosition;

    };

    typedef std::vector<Tool *> ToolsContainer;
    ToolsContainer Tools;

    //Class used to store surgical plan data
    class SurgicalPlan:public myGenericObject {
        public:
            SurgicalPlan():myGenericObject("SurgicalPlan") {this->entry.SetSize(3);
                                                            this->target.SetSize(3);
                                                            this->myStateTable.AddData(this->entry,this->myName + "entry");
                                                            this->myStateTable.AddData(this->target,this->myName + "target");}
            void Assign(const MNavStealthLink::DataItem & item_in);
            void ConfigureInterfaceProvided(mtsInterfaceProvided * provided_in);
            mtsDoubleVec entry;
            mtsDoubleVec target;
    };

    // Class used to store registration data
    class Registration: public myGenericObject
    {
    public:
        Registration():myGenericObject("Registration") {this->myStateTable.AddData(this->Transformation,this->myName + "Transformation");
                                                        this->myStateTable.AddData(this->Valid,this->myName + "Valid");
                                                        this->myStateTable.AddData(this->PredictedAccuracy,this->myName + "PredictedAccuracy");}

        void Assign(const MNavStealthLink::DataItem & item_in);
        void ConfigureInterfaceProvided(mtsInterfaceProvided * provided_in);

        mtsFrm3 Transformation;
        mtsBool Valid;
        mtsDouble PredictedAccuracy;


    };
    Registration RegistrationMember;
    void GetRegistrationValid(bool & valid_out) const;

    // Class used to store exam info
    class ExamInformation: public myGenericObject
    {
    public:
        ExamInformation():myGenericObject("ExamInformation"){this->myStateTable.AddData(this->VoxelScale,this->myName + "ExamInformationVoxelScale");
                                                             this->myStateTable.AddData(this->Size,this->myName + "ExamInformationSize");
                                                             this->myStateTable.AddData(this->Valid,this->myName + "ExamInformationValid");}
        void Assign(const MNavStealthLink::DataItem & item_in);
        void ConfigureInterfaceProvided(mtsInterfaceProvided * provided_in);

        mtsDouble3 VoxelScale;
        mtsInt3 Size;
        bool Valid;
    };

    void RequestExamInformation(void);

    /*! Mark all tool data as invalid */
    void ResetAllTools(void);

    /*! Find a tool using the stealh name */
    Tool * FindTool(const std::string & stealthName) const;

    /*! Add a tool using its stealth name and the name of the corresponding provided interface */
    Tool * AddTool(const std::string & stealthName, const std::string & interfaceName);

    void Init(void);

    // Stealthlink 2.0 callback

    class myCallback{
        private:
            mtsMedtronicStealthlink * my_parent;
        protected:
            template <typename T> friend class MNavStealthLink::Subscription;
            friend class mtsMedtronicStealthlink;
            void operator()(const MNavStealthLink::DataItem& item_in) const;
        public:
            myCallback(mtsMedtronicStealthlink * parent_in):my_parent(parent_in) {}
    };

    myCallback myCallbackMember;
    void LogWarning(const std::string & string_in){
        CMN_LOG_CLASS_RUN_WARNING << string_in  << std::endl;
    }

    //Stealthlink 2.0 Subscriptions

    MNavStealthLink::Subscription<MNavStealthLink::Instrument> * instrumentSubscription;
    MNavStealthLink::Subscription<MNavStealthLink::Frame> * frameSubscription;
    MNavStealthLink::Subscription<MNavStealthLink::Registration> * registrationSubscription;
    MNavStealthLink::Subscription<MNavStealthLink::SurgicalPlan> * surgicalPlanSubscription;

    struct less_DataItem : std::binary_function<const MNavStealthLink::DataItem *, const MNavStealthLink::DataItem *, bool>
    {
        bool operator() (const MNavStealthLink::DataItem * a, const MNavStealthLink::DataItem * b) const ;
    };

    typedef std::map<const MNavStealthLink::DataItem * ,myGenericObject *,less_DataItem> DataMapContainer;
    typedef std::pair<const MNavStealthLink::DataItem * ,myGenericObject *> DataMapContainerItem;
    typedef std::pair<DataMapContainer::iterator,bool> DataMapContainerInsertReturnValue;
    DataMapContainer myDataMap;

 public:
    mtsMedtronicStealthlink(const std::string & taskName);
    ~mtsMedtronicStealthlink();

    void Startup(void);

    /*! Configure the Stealthlink interface using an XML file. If the
      XML file is not found, the system uses a default IP address
      (192.168.0.1) and does not predefine any provided interfaces for
      the tools. Note that if a tool is not pre-defined via the XML
      file, it can still be discovered at runtime and a provided
      interface will be dynamically added.
    */
    void Configure(const std::string & filename = "");

    void Run(void);

    void Cleanup(void);
};

CMN_DECLARE_SERVICES_INSTANTIATION(mtsMedtronicStealthlink);



#endif // _mtsMedtronicStealthlink_h
