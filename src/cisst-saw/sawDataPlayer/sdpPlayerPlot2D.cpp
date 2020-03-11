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

#include "sdpPlayerPlot2D.h"


#include <math.h>
#include <QMenu>
#include <QFileDialog>
#include <QLabel>
#include <QDoubleSpinBox>
#include <cisstOSAbstraction/osaGetTime.h>


#include <iostream>
#include <sstream>

CMN_IMPLEMENT_SERVICES(sdpPlayerPlot2D);

sdpPlayerPlot2D::sdpPlayerPlot2D(const std::string & name, double period):
    sdpPlayerBase(name, period)
{

    // create the user interface
    ExWidget.setupUi(&Widget);
    mainWidget = new QWidget();
    ScaleZoom = new QDoubleSpinBox(mainWidget);
    ScaleZoom->setValue(1);
    ZoomInOut = new QLabel(mainWidget);
    ZoomInOut->setText("Set Visualization Scale");
    ScaleZoom->setMaximum(9999);

    // create the user interface
    Plot = new vctPlot2DOpenGLQtWidget(mainWidget);
    Plot->SetNumberOfPoints(100);
    SignalPointer = Plot->AddSignal("Data");
    VerticalLinePointer = Plot->AddVerticalLine("X");

    QLabel *upperLabel = new QLabel(mainWidget);
    upperLabel->setText("UpperY");
    QLabel *lowerLabel = new QLabel(mainWidget);
    lowerLabel->setText("LowerY");
    UpperYSpinBox  = new QDoubleSpinBox(&Widget);
    LowerYSpinBox  = new QDoubleSpinBox(&Widget);

    LowerYSpinBox->setValue(0);
    UpperYSpinBox->setValue(20);

    CentralLayout = new QGridLayout(mainWidget);

    CentralLayout->setContentsMargins(0, 0, 0, 0);
    CentralLayout->setRowStretch(0, 1);
    CentralLayout->setColumnStretch(1, 1);
    CentralLayout->addWidget(Plot, 0, 0, 1, 6);
    CentralLayout->addWidget(ZoomInOut, 1, 0, 1, 1);
    CentralLayout->addWidget(ScaleZoom, 1, 1, 1, 1);
    CentralLayout->addWidget(lowerLabel,1,2,1,1);
    CentralLayout->addWidget(LowerYSpinBox,1,3,1,1);
    CentralLayout->addWidget(upperLabel,1,4,1,1);
    CentralLayout->addWidget(UpperYSpinBox,1,5,1,1);

    CentralLayout->addWidget(this->GetWidget(),2,0,1,6);

    mainWidget->resize(300,500);

    // Add elements to state table
    StateTable.AddData(ZoomScaleValue,  "ZoomScale");
    StateTable.AddData(VectorIndex,  "VectorIndex");

    mtsInterfaceProvided * provided = AddInterfaceProvided("Provides2DPlot");
    if (provided) {
        provided->AddCommandReadState(StateTable, ZoomScaleValue,         "GetZoomScale");
        provided->AddCommandReadState(StateTable, VectorIndex,        "GetVectorIndex");
        provided->AddCommandWrite(&sdpPlayerPlot2D::SetVectorIndex, this, "SetVectorIndex", mtsInt() );
    }
    // Connect to ourself, for Qt Thread

    mtsInterfaceRequired * interfaceRequired = AddInterfaceRequired("Get2DPlotStatus");
    if (interfaceRequired) {
        interfaceRequired->AddFunction("GetZoomScale", Plot2DAccess.GetZoomScale);
        interfaceRequired->AddFunction("GetVectorIndex", Plot2DAccess.GetVectorIndex);
        interfaceRequired->AddFunction("SetVectorIndex",  Plot2DAccess.WriteVectorIndex);
    }

    ZoomScaleValue = 1;


    //// Add Parser Thread
    //taskManager = mtsTaskManager::GetInstance();
    //taskManager->AddComponent(&Parser);
}


sdpPlayerPlot2D::~sdpPlayerPlot2D()
{
    // cleanup
    //taskManager->KillAll();
    //taskManager->Cleanup();
}


void sdpPlayerPlot2D::MakeQTConnections(void)
{
    QObject::connect(ExWidget.PlayButton, SIGNAL(clicked()),
                     this, SLOT(QSlotPlayClicked()));

    QObject::connect(ExWidget.TimeSlider, SIGNAL(sliderMoved(int)),
                     this, SLOT(QSlotSeekSliderMoved(int)));

    QObject::connect(ExWidget.SyncCheck, SIGNAL(clicked(bool)),
                     this, SLOT(QSlotSyncCheck(bool)));

    QObject::connect(ExWidget.StopButton, SIGNAL(clicked()),
                     this, SLOT(QSlotStopClicked()));

    QObject::connect(ExWidget.SetSaveStartButton, SIGNAL(clicked()),
                     this, SLOT(QSlotSetSaveStartClicked()));

    QObject::connect(ExWidget.SetSaveEndButton, SIGNAL(clicked()),
                     this, SLOT(QSlotSetSaveEndClicked()));

    QObject::connect(ExWidget.OpenFileButton, SIGNAL(clicked()),
                     this, SLOT(QSlotOpenFileClicked()));

    QObject::connect(ScaleZoom , SIGNAL(valueChanged(double)),
                     this, SLOT(QSlotSpinBoxValueChanged(double)));


    QObject::connect(UpperYSpinBox , SIGNAL(valueChanged(double)),
                     this, SLOT(QSlotUpperYRangeSpinChanged(double)));

    QObject::connect(LowerYSpinBox , SIGNAL(valueChanged(double)),
                     this, SLOT(QSlotLowerYRangeSpinChanged(double)));


}


void sdpPlayerPlot2D::Configure(const std::string & CMN_UNUSED(filename))
{
    MakeQTConnections();
    Widget.setWindowTitle(QString::fromStdString(GetName()));
    Widget.show();
    mainWidget->show();
    ResetPlayer();
    // Start Parser Thread
    //taskManager->CreateAll();
    //taskManager->StartAll();
}


void sdpPlayerPlot2D::Startup(void)
{
    //BaseAccess.LoadData();
    //UpdateLimits();
}


void sdpPlayerPlot2D::Run(void)
{
    ProcessQueuedEvents();
    ProcessQueuedCommands();
    
    CS.Enter();
    //update the model (load data) etc.
    if (State == PLAY) {

        double currentTime = TimeServer.GetAbsoluteTimeInSeconds();
        Time = currentTime - PlayStartTime.Timestamp() + PlayStartTime.Data;

        if (Time.Data > PlayUntilTime.Data)  {
            Time = PlayUntilTime;
            State = STOP;
        }
        else {
            if (Parser.IsReady()) {
                if((ZoomScaleValue)  > (TimeBoundary-Time )*(0.8) && TimeBoundary <  PlayUntilTime.Data){
                    Parser.LoadDataFromFile(SignalPointer, Time, ZoomScaleValue, false);
                    //Parser.TriggerLoadDataFromFile(SignalPointer, Time, ZoomScaleValue, false);
                    Parser.GetBoundary(SignalPointer, TopBoundary, LowBoundary);
                    TimeBoundary  =TopBoundary;
                }
                // update plot
                UpdatePlot();
            }
        }
    }
    //make sure we are at the correct seek position.
    else if (State == SEEK) {
        if (Parser.IsReady()) {
            //// Everything here should be moved to Qt thread since we have to re-alloc a new Plot object
            //size_t i = 0;
            if(LastTime.Data != Time.Data ){
                LastTime = Time;
                PlayStartTime = Time;
                Parser.LoadDataFromFile(SignalPointer, Time, ZoomScaleValue, true);
                //Parser.TriggerLoadDataFromFile(SignalPointer, Time, ZoomScaleValue, true);
                Parser.GetBoundary(SignalPointer, TopBoundary, LowBoundary);
                TimeBoundary  = TopBoundary;
                // update plot
                UpdatePlot();
            }
        }
    }
    else if (State == STOP) {
        //do Nothing

        //// update plot
        //UpdatePlot();
    }
    
    CS.Leave();
    //now display updated data in the qt thread space.
    if (Widget.isVisible()) {
        emit QSignalUpdateQT();
    }
}


void sdpPlayerPlot2D::UpdatePlot(void)
{
    double ScaleValue = 0.0;
    mtsInt index;

    Plot2DAccess.GetVectorIndex(index);
    Plot2DAccess.GetZoomScale(ScaleValue);

    Plot->SetContinuousFitX(false);
    Plot->SetContinuousFitY(false);
    Plot->FitY(0, 20.0, 0);
    Plot->FitX(Time.Data-ScaleValue-PlayerDataInfo.DataStart() ,  Time.Data+ScaleValue-PlayerDataInfo.DataStart(), 0);
    VerticalLinePointer->SetX(Time.Data-PlayerDataInfo.DataStart());
    // UpdateGL should be called at Qt thread
}

//in QT thread space
void sdpPlayerPlot2D::UpdateQT(void)
{
    mtsDouble timevalue;
    CS.Enter();
    //BaseAccess.GetTime(timevalue);
    timevalue = Time;
    if (State == PLAY) {
        //Display the last datasample before Time.
        ExWidget.TimeSlider->setValue((int)timevalue.Data);
        //update Plot in Qt Thread
        if(Plot)
            Plot->updateGL();
    }
    else if (State == STOP) {
        //Optional: Test if the data needs to be updated:
        ExWidget.TimeSlider->setValue((int)timevalue.Data);
        //update Plot in Qt Thread
        if(Plot)
            Plot->updateGL();
    }
    else if (State == SEEK) {
        //Optional: Test if the data needs to be updated:
        ExWidget.TimeSlider->setValue((int)timevalue.Data);
        //update Plot in Qt Thread
        if(Plot)
            Plot->updateGL();
    }
    CS.Leave();
    ExWidget.TimeLabel->setText(QString::number(timevalue.Data,'f', 3));
}


void sdpPlayerPlot2D::Play(const mtsDouble & time)
{
    if (Sync) {
        CMN_LOG_CLASS_RUN_DEBUG << "Play " << PlayStartTime << std::endl;
        State = PLAY;
        PlayUntilTime = PlayerDataInfo.DataEnd();
        PlayStartTime = time;
    }
}


void sdpPlayerPlot2D::Stop(const mtsDouble & time)
{
    if (Sync) {
        CMN_LOG_CLASS_RUN_DEBUG << "Stop " << time << std::endl;
        PlayUntilTime = time;
    }
}


void sdpPlayerPlot2D::Seek(const mtsDouble & time)
{
    static mtsDouble lasttime =0 ;
    if (Sync && lasttime.Data != time.Data) {
        CMN_LOG_CLASS_RUN_DEBUG << "Seek " << time << std::endl;

        State = SEEK;
        PlayUntilTime = PlayerDataInfo.DataEnd();
        // this will cause state table write command overflow
        //BaseAccess.WriteTime(time);
        Time = time;
        lasttime = time;
    }
}


void sdpPlayerPlot2D::Save(const sdpSaveParameters & saveParameters)
{
    if (Sync) {
        CMN_LOG_CLASS_RUN_DEBUG << "Save " << saveParameters << std::endl;
    }
}


void sdpPlayerPlot2D::Quit(void)
{
    CMN_LOG_CLASS_RUN_DEBUG << "Quit" << std::endl;
    this->Kill();
}


void sdpPlayerPlot2D::QSlotPlayClicked(void)
{
    mtsDouble playTime;
    BaseAccess.GetTime(playTime);
    playTime.Timestamp() = TimeServer.GetAbsoluteTimeInSeconds();

    if (Sync) {
        PlayRequest(playTime);
    } else {
        //not quite thread safe, if there is mts play call this can be corrupt.
        State = PLAY;
        PlayUntilTime = PlayerDataInfo.DataEnd();
        PlayStartTime = playTime;
    }
}


void sdpPlayerPlot2D::QSlotSeekSliderMoved(int c)
{
    mtsDouble t = c;
    static mtsDouble lasttime;
    if(lasttime.Data == t.Data)
        return;
    else
        lasttime = t;

    if (Sync) {
        SeekRequest(t);
    }
    State = SEEK;
    Time = t ;
    PlayUntilTime = PlayerDataInfo.DataEnd();
}


void sdpPlayerPlot2D::QSlotSyncCheck(bool checked)
{
    Sync = checked;
}


void sdpPlayerPlot2D::QSlotStopClicked(void)
{
    mtsDouble now = Time;

    if (Sync) {
        StopRequest(now);
    } else {
        PlayUntilTime = now;
    }
}


void sdpPlayerPlot2D::LoadData(void)
{
    //PlayerDataInfo.DataStart() = 1297723451.415;
    //PlayerDataInfo.DataEnd() = 1297723900.022;

    OpenFile();

    if (Time.Data < PlayerDataInfo.DataStart()) {
        Time = PlayerDataInfo.DataStart();
    }

    if (Time.Data > PlayerDataInfo.DataEnd()) {
        Time = PlayerDataInfo.DataEnd();
    }

    //This is the standard.
    PlayUntilTime = PlayerDataInfo.DataEnd();
    Time =  PlayerDataInfo.DataStart();

    UpdatePlayerInfo(PlayerDataInfo);
}


void sdpPlayerPlot2D::QSlotSetSaveStartClicked(void)
{
    mtsDouble timevalue;
    BaseAccess.GetTime(timevalue);
    ExWidget.SaveStartSpin->setValue(timevalue.Data);
}


void sdpPlayerPlot2D::QSlotSetSaveEndClicked(void)
{
    mtsDouble timevalue;
    BaseAccess.GetTime(timevalue);
    ExWidget.SaveEndSpin->setValue(timevalue.Data);
}


void sdpPlayerPlot2D::QSlotOpenFileClicked(void)
{
    QString fileName = QFileDialog::getOpenFileName(mainWidget, "Open File", tr("./"), tr("Desc (*.desc)"));
    FileName = fileName.toStdString();

    if (fileName.isEmpty()) {
        CMN_LOG_CLASS_RUN_WARNING<<"File not selected, no data to load"<<std::endl;
        return;
    }

    // read data and update relative times
    BaseAccess.LoadData();

}


// Executed in Qt Thread
void sdpPlayerPlot2D::QSlotSpinBoxValueChanged(double value)
{
    ZoomScaleValue = value;
    ScaleZoom->setValue(ZoomScaleValue);
    UpdatePlot();
}


// read data from file
void sdpPlayerPlot2D::OpenFile(void)
{
    if (!FileName.empty()) {

        // read Data from file
        ExtractDataFromStateTableCSVFile(FileName);

        Parser.GetBoundary(SignalPointer,TopBoundary,LowBoundary);
        TimeBoundary = TopBoundary;
        ResetPlayer();
        UpdatePlot();
        BaseAccess.WriteTime(LowBoundary);
        UpdateLimits();
    }
}


void sdpPlayerPlot2D::UpdateLimits()
{
    ExWidget.TimeSlider->setRange((int)PlayerDataInfo.DataStart(), (int)PlayerDataInfo.DataEnd());

    ExWidget.TimeStartLabel->setText(QString::number(PlayerDataInfo.DataStart(),'f', 3));
    ExWidget.TimeEndLabel->setText(QString::number(PlayerDataInfo.DataEnd(),'f', 3));

    ExWidget.SaveStartSpin->setRange(PlayerDataInfo.DataStart(), PlayerDataInfo.DataEnd());
    ExWidget.SaveEndSpin->setRange(PlayerDataInfo.DataStart(), PlayerDataInfo.DataEnd());
    ExWidget.SaveEndSpin->setValue(PlayerDataInfo.DataEnd());

}


bool sdpPlayerPlot2D::ExtractDataFromStateTableCSVFile(std::string & path){

    const std::string TimeFieldName("TimeStamp");
    const std::string DataFieldName("TipForceNorm_Nm");
    std::string Path(path);

    // open header file
    Parser.ParseHeader(Path);
    Parser.GenerateIndex();
    // we sould name the file Path - .desc + .idx
    Parser.WriteIndexToFile("Parser.idx");
    Parser.SetDataFieldForSearch(DataFieldName);
    Parser.SetTimeFieldForSearch(TimeFieldName);
    Parser.LoadDataFromFile(SignalPointer, 0.0, ZoomScaleValue,  false);
    //Parser.TriggerLoadDataFromFile(SignalPointer, 0.0, ZoomScaleValue,  false);

    Parser.GetBoundary(SignalPointer,TopBoundary,LowBoundary);

    Parser.GetBeginEndTime(PlayerDataInfo.DataStart(), PlayerDataInfo.DataEnd());
    //    Data = &DataPool1;
    //    TimeStamps =&TimeStampsPool1;

    return true;
}

// reset player to initial state
//! TODO: make this function thread safe
void sdpPlayerPlot2D::ResetPlayer(void)
{    
    // set to maximun period we read
    //ZoomScaleValue = (PlayerDataInfo.DataStart() != 0) ? ((PlayerDataInfo.DataEnd() - PlayerDataInfo.DataStart()) / 2.0) : 1.0 ;
    //if(TimeStamps->size() != 0)
    //    ZoomScaleValue = (TimeStamps->at(TimeStamps->size()-1) - TimeStamps->at(0))/2.0;

    ScaleZoom->setValue(ZoomScaleValue);
    BaseAccess.WriteTime(0.0);
    Plot2DAccess.WriteVectorIndex(0);
}


void sdpPlayerPlot2D::SetSynced(bool isSynced) {

    ExWidget.SyncCheck->setChecked(isSynced);
    Sync = isSynced;

}


void sdpPlayerPlot2D::QSlotUpperYRangeSpinChanged(double upperRange) {
    CMN_LOG_CLASS_RUN_VERBOSE << "Upper Y Range update:" << upperRange<<std::endl;
    vct2 r = Plot->GetViewingRangeY();
    Plot->FitY(r[0], upperRange, 0);
}

void sdpPlayerPlot2D::QSlotLowerYRangeSpinChanged(double lowerRange) {
    CMN_LOG_CLASS_RUN_VERBOSE << "Lower Y Range update:" << lowerRange << std::endl;
    vct2 r = Plot->GetViewingRangeY();
    Plot->FitY(lowerRange, r[1], 0);
}
