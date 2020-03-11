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

#include "sdpPlayerAudio.h"
#include <math.h>
#include <QMenu>
#include <QGridLayout>
#include <iostream>
#include <sstream>
#include <QFileDialog>

#include <cisstOSAbstraction/osaGetTime.h>
#include <cisstStereoVision/svlFilterOutput.h>
#include <QMessageBox>


CMN_IMPLEMENT_SERVICES(sdpPlayerAudio);

sdpPlayerAudio::sdpPlayerAudio(const std::string & name, double period):
    sdpPlayerBase(name,period)
{
    ExWidget.setupUi(&Widget);

    // create the widgets
    Plot        = new vctPlot2DOpenGLQtWidget(&Widget);
    DataTrace   = Plot->AddSignal("VolumeData");

    VolumeProgressBar = new QProgressBar(&Widget);
    VolumeProgressBar->setObjectName(QString::fromUtf8("VolumeProgressBar"));
    VolumeProgressBar->setValue(0);
    VolumeProgressBar->setTextVisible(false);
    VolumeProgressBar->setOrientation(Qt::Vertical);
    VolumeProgressBar->setInvertedAppearance(false);
    VolumeProgressBar->setTextDirection(QProgressBar::BottomToTop);


    VolumeSlider = new QSlider(&Widget);
    VolumeSlider->setObjectName(QString::fromUtf8("VolumeSlider"));
    VolumeSlider->setLayoutDirection(Qt::LeftToRight);
    VolumeSlider->setMaximum(100);
    VolumeSlider->setValue(50);
    VolumeSlider->setOrientation(Qt::Vertical);
    VolumeSlider->setTickPosition(QSlider::TicksBothSides);

    QGridLayout *CentralLayout = new QGridLayout(&MainWindow);
    CentralLayout->setContentsMargins(0, 0, 0, 0);
    CentralLayout->setRowStretch(0, 1);
    CentralLayout->setColumnStretch(1, 1);

//    label = new QLabel(&Widget);
//    label->setObjectName(QString::fromUtf8("label"));
//    label->setAlignment(Qt::AlignCenter);
//    label->setText("Vol");

    SliderVLayout = new QVBoxLayout();
    SliderVLayout->addWidget(VolumeSlider);
//    SliderVLayout->addWidget(label);

    HorizontalLayout = new QHBoxLayout();
    HorizontalLayout->addWidget(Plot);
    HorizontalLayout->addWidget(VolumeProgressBar);
    HorizontalLayout->addLayout(SliderVLayout);
    HorizontalLayout->setAlignment(Qt::AlignRight);

    CentralLayout->addLayout(HorizontalLayout, 0, 0, 1, 4);
    CentralLayout->addWidget(&Widget,1,1,1,1);

}


sdpPlayerAudio::~sdpPlayerAudio()
{
}


void sdpPlayerAudio::MakeQTConnections(void)
{
    QObject::connect(ExWidget.PlayButton, SIGNAL(clicked()),
                     this, SLOT( QSlotPlayClicked()) );

    QObject::connect(ExWidget.TimeSlider, SIGNAL(sliderMoved(int)),
                     this, SLOT( QSlotSeekSliderMoved(int)) );

    QObject::connect(ExWidget.SyncCheck, SIGNAL(clicked(bool)),
                     this, SLOT( QSlotSyncCheck(bool)) );

    QObject::connect(ExWidget.StopButton, SIGNAL(clicked()),
                     this, SLOT( QSlotStopClicked()) );

    QObject::connect(ExWidget.SetSaveStartButton, SIGNAL(clicked()),
                     this, SLOT( QSlotSetSaveStartClicked()) );

    QObject::connect(ExWidget.SetSaveEndButton, SIGNAL(clicked()),
                     this, SLOT( QSlotSetSaveEndClicked()) );

    QObject::connect(ExWidget.OpenFileButton, SIGNAL(clicked()),
                     this, SLOT( QSlotOpenFileClicked()) );

    QObject::connect(VolumeSlider, SIGNAL(sliderMoved(int)),
                     this, SLOT(QSlotVolumeSliderMoved(int)));

    QObject::connect(ExWidget.SetRangeButton, SIGNAL(clicked()),
                     this, SLOT( QSlotSetRangeClicked()) );

}


void sdpPlayerAudio::Configure(const std::string & CMN_UNUSED(filename))
{
    MakeQTConnections();

    //Widget.setWindowTitle(QString::fromStdString(GetName()));
    // Widget.show();
    MainWindow.setWindowTitle(QString::fromStdString(GetName()));
    MainWindow.resize(300,200);
    MainWindow.show();
}


void sdpPlayerAudio::Startup(void)
{
    Audio.Startup();

}


void sdpPlayerAudio::Run(void)
{
    ProcessQueuedEvents();
    ProcessQueuedCommands();

    //update the model (load data) etc.
    if (State == PLAY) {

        double currentTime = TimeServer.GetAbsoluteTimeInSeconds();
        Time = currentTime - PlayStartTime.Timestamp() + PlayStartTime;

        if (Time > PlayUntilTime) {
            Time = PlayUntilTime;
            State = STOP;
        }
        else {
            //Load and Prep current data
            //CMN_LOG_CLASS_RUN_WARNING<<"pos: "<<source.GetPositionAtTime(Time.Data)<<std::endl;
            //CMN_LOG_CLASS_RUN_WARNING<<"at T: "<<source.GetTimeAtPosition(source.GetPositionAtTime(Time.Data))<<std::endl;
            Audio.Seek(Time);
            Audio.Play();
        }
    }
    //make sure we are at the correct seek position.
    else if (State == SEEK) {
        //Load and Prep current data
        // CMN_LOG_CLASS_RUN_WARNING<<"pos: "<<source.GetPositionAtTime(Time.Data)<<std::endl;
        //CMN_LOG_CLASS_RUN_WARNING<<"at T: "<<source.GetTimeAtPosition(source.GetPositionAtTime(Time.Data))<<std::endl;
        Audio.Seek(Time);
        //Audio.Play();
    }

    else if (State == STOP) {
        //do Nothing
        Audio.Pause();
        // CMN_LOG_CLASS_RUN_WARNING<<"pos: "<<source.GetPositionAtTime(Time.Data)<<std::endl;
        // CMN_LOG_CLASS_RUN_WARNING<<"at T: "<<source.GetTimeAtPosition(source.GetPositionAtTime(Time.Data))<<std::endl;
    }

    Audio.Run();

    //now display updated data in the qt thread space.
    if (Widget.isVisible()) {
        emit QSignalUpdateQT();
    }
}


//in QT thread space
void sdpPlayerAudio::UpdateQT(void)
{
    if (State == PLAY) {
        //Display the last datasample before Time.
        ExWidget.TimeSlider->setValue((int)Time.Data);

        mtsDouble v;
        Audio.GetStreamVolume(v);
        VolumeProgressBar->setValue(v.Data * 100);
        //the plot widget probably uses floats which does not allow for plotting timestamped data.
        DataTrace->AppendPoint(vctDouble2(v.Timestamp() -  PlayerDataInfo.DataStart(), v.Data));
        //std::cout<<v.Data<< "   "<<v.Timestamp() -  PlayerDataInfo.DataStart()<<std::endl;
        Plot->updateGL();

    }
    //Make sure we are at the correct seek location.
    else if (State == STOP) {
        //Optional: Test if the data needs to be updated:
        ExWidget.TimeSlider->setValue((int)Time.Data);
    }

    else if (State == SEEK) {
        //Optional: Test if the data needs to be updated:
        ExWidget.TimeSlider->setValue((int)Time.Data);
    }

    ExWidget.TimeLabel->setText(QString::number(Time.Data,'f', 3));
}


void sdpPlayerAudio::Play(const mtsDouble & time)
{
    if (Sync) {
        CMN_LOG_CLASS_RUN_DEBUG << "Play " << PlayStartTime << std::endl;
        State = PLAY;
        PlayUntilTime = PlayerDataInfo.DataEnd();
        PlayStartTime = time;
    }
}


void sdpPlayerAudio::Stop(const mtsDouble & time)
{
    if (Sync) {
        CMN_LOG_CLASS_RUN_DEBUG << "Stop " << time << std::endl;
        PlayUntilTime = time;
    }
}


void sdpPlayerAudio::Seek(const mtsDouble & time)
{
    if (Sync) {
        CMN_LOG_CLASS_RUN_DEBUG << "Seek " << time << std::endl;

        State = SEEK;
        PlayUntilTime = PlayerDataInfo.DataEnd();
        Time = time;
    }
}


void sdpPlayerAudio::Save(const sdpSaveParameters & saveParameters)
{

     CMN_LOG_CLASS_RUN_ERROR << "Save " << saveParameters << std::endl;
    if (Sync) {
        CMN_LOG_CLASS_RUN_VERBOSE << "Save " << saveParameters << std::endl;

        std::stringstream audiofilenameStrm;

        audiofilenameStrm << saveParameters.Path().Data <<saveParameters.Prefix().Data;

        CMN_LOG_CLASS_RUN_VERBOSE << "`Save`: setting up save audio file with prefix: `" << audiofilenameStrm.str() << "`" << std::endl;

        Audio.SaveClip(audiofilenameStrm.str(),saveParameters.Start(), saveParameters.End());
        //QMessageBox::critical(this->GetWidget(), tr(GetName().c_str()),tr("Saved Audio file"));

    }
}


void sdpPlayerAudio::Quit(void)
{
    CMN_LOG_CLASS_RUN_DEBUG << "Quit" << std::endl;
    this->Kill();
}


void sdpPlayerAudio::QSlotPlayClicked(void)
{
    mtsDouble playTime = Time; //this should be read from the state table!!!
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


void sdpPlayerAudio::QSlotSeekSliderMoved(int c)
{
    mtsDouble t = c;

    if (Sync) {
        SeekRequest(t);
    } else {
        State = SEEK;
        Time = t;
    }
    PlayUntilTime = PlayerDataInfo.DataEnd();
}


void sdpPlayerAudio::QSlotSyncCheck(bool checked)
{
    Sync = checked;
}


void sdpPlayerAudio::QSlotStopClicked(void)
{
    mtsDouble now = Time;

    if (Sync) {
        StopRequest(now);
    } else {
        PlayUntilTime = now;
    }
}


void sdpPlayerAudio::LoadData(void)
{

    Audio.OpenFile(FileName);

    PlayerDataInfo.DataStart()  = Audio.GetStartTime();
    PlayerDataInfo.DataEnd()    = Audio.GetEndTime();

    //make sure we are within bounds to start with.
    if (Time < PlayerDataInfo.DataStart()) {
        Time = PlayerDataInfo.DataStart();
    }

    if (Time > PlayerDataInfo.DataEnd()) {
        Time = PlayerDataInfo.DataEnd();
    }

    //This is the standard.
    PlayUntilTime = PlayerDataInfo.DataEnd();

    UpdatePlayerInfo(PlayerDataInfo);
    UpdateLimits();
}


void sdpPlayerAudio::QSlotSetSaveStartClicked(void)
{
    ExWidget.SaveStartSpin->setValue(Time.Data);
}


void sdpPlayerAudio::QSlotSetSaveEndClicked(void)
{
    ExWidget.SaveEndSpin->setValue(Time.Data);
}

void sdpPlayerAudio::QSlotOpenFileClicked(void){

    QString fileName = QFileDialog::getOpenFileName(&Widget, tr("Select video file"),tr("./"),tr("Audio (*.wav)"));

    FileName = fileName.toStdString();
    if (FileName.empty()) {
        CMN_LOG_CLASS_RUN_WARNING<<"File not selected, no data to load"<<std::endl;
        return;
    }
    BaseAccess.LoadData();

}

void sdpPlayerAudio::UpdateLimits()
{
    ExWidget.TimeSlider->setRange((int)PlayerDataInfo.DataStart(), (int)PlayerDataInfo.DataEnd());

    ExWidget.TimeStartLabel->setText( QString::number(PlayerDataInfo.DataStart(),'f', 3));
    ExWidget.TimeEndLabel->setText( QString::number( PlayerDataInfo.DataEnd(),'f', 3));

    ExWidget.SaveStartSpin->setRange( PlayerDataInfo.DataStart(), PlayerDataInfo.DataEnd());
    ExWidget.SaveEndSpin->setRange( PlayerDataInfo.DataStart(), PlayerDataInfo.DataEnd());
    ExWidget.SaveEndSpin->setValue(PlayerDataInfo.DataEnd());

}

void sdpPlayerAudio::SetSynced(bool isSynced) {

    ExWidget.SyncCheck->setChecked(isSynced);
    Sync = isSynced;
}


void sdpPlayerAudio::QSlotVolumeSliderMoved(int v) {

    mtsDouble vol(v/100.0);
    Audio.SetVolume(vol);
}

void sdpPlayerAudio::QSlotSetRangeClicked(void) {

      SaveParameters.Start() = ExWidget.SaveStartSpin->value();
      SaveParameters.End() = ExWidget.SaveEndSpin->value();

      UpdateSaveParams(SaveParameters);
}

