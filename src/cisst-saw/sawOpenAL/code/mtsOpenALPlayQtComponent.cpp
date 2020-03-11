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

#include <QMessageBox>
#include <QTableWidgetItem>
#include <QStringList>
#include <QList>
#include <QFileDialog>

#include <sawOpenAL/mtsOpenALPlayQtComponent.h>
#include <cisstMultiTask/mtsInterfaceRequired.h>
#include <cisstMultiTask/mtsTaskManager.h>

CMN_IMPLEMENT_SERVICES(mtsOpenALPlayQtComponent);

mtsOpenALPlayQtComponent::mtsOpenALPlayQtComponent(const std::string & name, double period):
    mtsComponent(name),
    Time(0),
    DataStartTime(0),
    DataEndTime(0),
    PlayStartTime(0)
{
    startTimer(period*1000);
    ErrorMessageDialog = new QErrorMessage();

    // create the user interface
    PlayWidget.setupUi(&Widget);

    // create the widgets
    Plot        = new vctPlot2DOpenGLQtWidget(&Widget);
    Plot->SetNumberOfPoints(400);

    DataTrace   = Plot->AddSignal("VolumeData");

    PlayWidget.MainVLayout->addWidget(Plot);

    SeekSlider  = new QSlider( Qt::Horizontal, &Widget);

    PlayWidget.SliderVLayout->addWidget(SeekSlider);

    TimeServer = mtsTaskManager::GetInstance()->GetTimeServer();

    mtsInterfaceRequired *required = AddInterfaceRequired("RequiresAudioPlayer", MTS_OPTIONAL);
    if (required) {
        required->AddFunction("Play",           Player.Play);
        required->AddFunction("Pause",          Player.Pause);
        required->AddFunction("Seek",           Player.Seek);
        required->AddFunction("GetStartTime",   Player.GetStartTime);
        required->AddFunction("OpenFile",       Player.OpenFile);
        required->AddFunction("GetIsPlaying",   Player.GetIsPlaying);
        required->AddFunction("GetVolume",      Player.GetVolume);
        required->AddFunction("SetVolume",      Player.SetVolume);
        required->AddFunction("GetTime",        Player.GetTime);
        required->AddFunction("GetLengthInSec", Player.GetLengthInSec);
        required->AddFunction("GetStreamVolume",Player.GetStreamVolume);
        required->AddEventHandlerVoid(&mtsOpenALPlayQtComponent::RangeChangedEvent,this, "RangeChangedEvent", MTS_EVENT_NOT_QUEUED);
    }
}


void mtsOpenALPlayQtComponent::MakeQTConnections(void)
{
    QObject::connect(PlayWidget.PlayButton, SIGNAL(clicked()),
                     this, SLOT(QSlotPlayClicked()));

    QObject::connect(PlayWidget.PauseButton, SIGNAL(clicked()),
                     this, SLOT(QSlotPauseClicked()));
    QObject::connect(PlayWidget.FileDialogButton, SIGNAL(clicked()),
                     this, SLOT(QSlotFileDialogClicked()));

    QObject::connect(PlayWidget.OpenFileButton, SIGNAL(clicked()),
                     this, SLOT(QSlotOpenFileClicked()));

    QObject::connect(PlayWidget.VolumeSlider, SIGNAL(sliderMoved(int)),
                     this, SLOT(QSlotVolumeSliderMoved(int)));


    QObject::connect(this->SeekSlider, SIGNAL(sliderMoved(int)),
                     this, SLOT(QSlotSeekSliderMoved(int)));

    QObject::connect(this, SIGNAL(QSignalUpdateRange()),
                     this, SLOT(QSlotUpdateRange()));
}


void mtsOpenALPlayQtComponent::Configure(const std::string & CMN_UNUSED(filename))
{
    MakeQTConnections();
    //Widget.resize(700,300);
    Widget.show();
}


void mtsOpenALPlayQtComponent::ErrorMessage(const std::string & message)
{
    QMessageBox::critical(this->GetWidget(), tr(GetName().c_str()), tr(message.c_str()));
}


void mtsOpenALPlayQtComponent::QSlotPlayClicked()
{
    Player.Play();
}


void mtsOpenALPlayQtComponent::QSlotPauseClicked()
{
    Player.Pause();
}


void mtsOpenALPlayQtComponent::timerEvent(QTimerEvent * )
{
    //PlayWidget.TimeLabel->setText(QString::number(Time.Data,'f', 3));
    mtsBool isPlaying;
    Player.GetIsPlaying(isPlaying);
    if (isPlaying) {
        //        PlayWidget.IsPlayingLabel->setStyleSheet("QLabel { background-color : green; color : white; }");
        //        PlayWidget.IsPlayingLabel->setText("PLAYING");
        mtsDouble v;
        Player.GetStreamVolume(v);
        PlayWidget.VolumeProgressBar->setValue(v.Data * 100);
        //the plot widget probably uses floats which does not allow for plotting timestamped data.
        DataTrace->AppendPoint(vctDouble2(v.Timestamp() - DataStartTime, v.Data));
        //std::cout<<v<<std::endl;
        Plot->updateGL();
    }
    else {
        //        PlayWidget.IsPlayingLabel->setStyleSheet("QLabel { background-color : red; color : black; }");
        //        PlayWidget.IsPlayingLabel->setText("STOPPED");
    }

    mtsDouble t;
    Player.GetTime(t);
    PlayWidget.TimeLabel->setText(QString::number(t.Data, 'f', 3));
    if (!SeekSlider->isSliderDown())
        SeekSlider->setValue((int)(t.Data - DataStartTime));
}


void mtsOpenALPlayQtComponent::QSlotFileDialogClicked()
{
    QString fileName = QFileDialog::getOpenFileName(&Widget, tr("Select audio file"), tr("./"), tr("Audio (*.cai *.wav)"));
    PlayWidget.FileLineEdit->setText(fileName);
    QSlotOpenFileClicked();
}


void mtsOpenALPlayQtComponent::QSlotOpenFileClicked()
{
    Player.OpenFile(mtsStdString(PlayWidget.FileLineEdit->text().toStdString()));
}


void mtsOpenALPlayQtComponent::QSlotVolumeSliderMoved(int v)
{
    mtsDouble vol(v/100.0);
    Player.SetVolume(vol);
}


void mtsOpenALPlayQtComponent::QSlotSeekSliderMoved(int v)
{
    //Player.Pause();
    Player.Seek(mtsDouble(DataStartTime + v));

}


void mtsOpenALPlayQtComponent::QSlotUpdateRange()
{
    mtsDouble tLength;
    Player.GetStartTime(DataStartTime);
    Player.GetLengthInSec(tLength);
    DataEndTime = DataStartTime + tLength;
    SeekSlider->setRange(0, (int) (DataEndTime - DataStartTime));
}

