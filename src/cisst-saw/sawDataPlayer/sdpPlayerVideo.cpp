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

#include "sdpPlayerVideo.h"
#include <math.h>
#include <QMenu>
#include <QGridLayout>
#include <iostream>
#include <sstream>
#include <QFileDialog>
#include <cisstStereoVision/svlFilterVideoFileWriter.h>
#include <cisstStereoVision/svlFilterImageFileWriter.h>
#include <cisstStereoVision/svlFilterImageChannelSwapper.h>

#include <cisstOSAbstraction/osaGetTime.h>
#include <cisstStereoVision/svlFilterOutput.h>
#include <cisstOSAbstraction/osaSleep.h>
#include <cisstStereoVision/svlFilterImageCropper.h>


CMN_IMPLEMENT_SERVICES(sdpPlayerVideo);

sdpPlayerVideo::sdpPlayerVideo(const std::string & name, double period):
    sdpPlayerBase(name,period),
    StreamManager(2),
    TimestampOverlay(0)
{
    ExWidget.setupUi(&Widget);
    VideoWidget = new svlFilterImageOpenGLQtWidget();
    VideoWidget->SetEnableToolTip(true);

    QGridLayout *CentralLayout = new QGridLayout(&MainWindow);
    CentralLayout->setContentsMargins(0, 0, 0, 0);
    CentralLayout->setRowStretch(0, 1);
    CentralLayout->setColumnStretch(1, 1);
    CentralLayout->setObjectName("CentralLayout");

    CentralLayout->addWidget(VideoWidget, 0, 0, 1, 4);
    CentralLayout->addWidget(&Widget,1,1,1,1);
    Cropper.Enable();

    CropButton = new QCheckBox(&Widget);
    CropButton->setCheckable(true);
    CropButton->setChecked(false);
    CropButton->setText("Crop");

    SwapRGB_Button = new QCheckBox(&Widget);
    SwapRGB_Button->setCheckable(true);
    SwapRGB_Button->setChecked(false);
    SwapRGB_Button->setText("SwapRGB");

    SaveAsBMPButton = new QCheckBox(&Widget);
    SaveAsBMPButton->setCheckable(true);
    SaveAsBMPButton->setChecked(false);
    SaveAsBMPButton->setText("SaveBMPs");

    LeftSpinBox     = new QSpinBox(&Widget);
    RightSpinBox    = new QSpinBox(&Widget);
    TopSpinBox      = new QSpinBox(&Widget);
    BottomSpinBox   = new QSpinBox(&Widget);

    QGridLayout *cropLayout = new QGridLayout(&Widget);
    cropLayout->setObjectName("croplayout");

    CentralLayout->addLayout(cropLayout,1,3,1,1);
    cropLayout->addWidget(TopSpinBox,0,0,1,2,Qt::AlignJustify);
    cropLayout->addWidget(LeftSpinBox,1,0,1,1,Qt::AlignLeft);
    cropLayout->addWidget(RightSpinBox,1,1,1,1,Qt::AlignRight);
    cropLayout->addWidget(BottomSpinBox,2,0,1,2,Qt::AlignJustify);
    cropLayout->addWidget(CropButton,3,1,1,1,Qt::AlignJustify);
    cropLayout->addWidget(SwapRGB_Button,3,0,1,1,Qt::AlignJustify);
    cropLayout->addWidget(SaveAsBMPButton,4,0,1,1,Qt::AlignJustify);

    QApplication::instance()->installEventFilter(this);
    //Widget.installEventFilter(this);

}


sdpPlayerVideo::~sdpPlayerVideo()
{
}


void sdpPlayerVideo::MakeQTConnections(void)
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

    QObject::connect(ExWidget.SetRangeButton, SIGNAL(clicked()),
                     this, SLOT( QSlotSetRangeClicked()) );

    QObject::connect(this->CropButton, SIGNAL(clicked(bool)),
                     this, SLOT( QSlotCropButtonClicked(bool)) );
    QObject::connect(this->SwapRGB_Button, SIGNAL(clicked()),
                     VideoWidget, SLOT( QSlotSwapRGB()) );
}


void sdpPlayerVideo::Configure(const std::string & CMN_UNUSED(filename))
{
    MakeQTConnections();


    //Widget.setWindowTitle(QString::fromStdString(GetName()));
    // Widget.show();
    MainWindow.setWindowTitle(QString::fromStdString(GetName()));
    MainWindow.resize(300,500);
    MainWindow.show();
    //    LoadData();
    //  UpdateLimits();

}


void sdpPlayerVideo::Startup(void)
{


}


void sdpPlayerVideo::Run(void)
{
    ProcessQueuedEvents();
    ProcessQueuedCommands();

    //update the model (load data) etc.
    if (State == PLAY) {

        double currentTime = TimeServer.GetAbsoluteTimeInSeconds();
        Time = currentTime - PlayStartTime.Timestamp() + PlayStartTime.Data;

        if (Time > PlayUntilTime) {
            Time = PlayUntilTime;
            State = STOP;
        }
        else {
            //Load and Prep current data
            //source.Play();
            //CMN_LOG_CLASS_RUN_WARNING<<"pos: "<<Source.GetPositionAtTime(Time.Data)<<std::endl;
            //CMN_LOG_CLASS_RUN_WARNING<<"at T: "<<source.GetTimeAtPosition(source.GetPositionAtTime(Time.Data))<<std::endl;
            if ( Source.IsInitialized()) {
                Source.SetPosition(Source.GetPositionAtTime(Time.Data));
                Source.Play();
            }
        }
    }
    //make sure we are at the correct seek position.
    else if (State == SEEK) {
        //Load and Prep current data
        // CMN_LOG_CLASS_RUN_WARNING<<"pos: "<<source.GetPositionAtTime(Time.Data)<<std::endl;
        //CMN_LOG_CLASS_RUN_WARNING<<"at T: "<<Source.GetTimeAtPosition(Source.GetPositionAtTime(Time.Data))<<std::endl;
        if ( Source.IsInitialized()) {
            Source.SetPosition(Source.GetPositionAtTime(Time.Data));
            Source.Play();

        }
    }
    else if (State == STOP) {
        //do Nothing
        if ( Source.IsInitialized())
            Source.Pause();
        // CMN_LOG_CLASS_RUN_WARNING<<"pos: "<<source.GetPositionAtTime(Time.Data)<<std::endl;
        // CMN_LOG_CLASS_RUN_WARNING<<"at T: "<<source.GetTimeAtPosition(source.GetPositionAtTime(Time.Data))<<std::endl;
    }

    //now display updated data in the qt thread space.
    if (Widget.isVisible()) {
        emit QSignalUpdateQT();
    }
}


//in QT thread space
void sdpPlayerVideo::UpdateQT(void)
{
    if (State == PLAY) {
        //Display the last datasample before Time.
        ExWidget.TimeSlider->setValue((int)Time.Data);
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


void sdpPlayerVideo::Play(const mtsDouble & time)
{
    if (Sync) {
        CMN_LOG_CLASS_RUN_DEBUG << "Play " << PlayStartTime << std::endl;
        State = PLAY;
        PlayUntilTime = PlayerDataInfo.DataEnd();
        PlayStartTime = time;
    }
}


void sdpPlayerVideo::Stop(const mtsDouble & time)
{
    if (Sync) {
        CMN_LOG_CLASS_RUN_DEBUG << "Stop " << time << std::endl;
        PlayUntilTime = time;
    }
}


void sdpPlayerVideo::Seek(const mtsDouble & time)
{
    if (Sync) {
        CMN_LOG_CLASS_RUN_DEBUG << "Seek " << time << std::endl;

        State = SEEK;
        PlayUntilTime = PlayerDataInfo.DataEnd();
        Time = time;
    }
}


void sdpPlayerVideo::Save(const sdpSaveParameters & saveParameters)
{
    //std::cout<< "Save " << saveParameters << std::endl;
    if (Sync) {
        CMN_LOG_CLASS_RUN_VERBOSE << "Save " << saveParameters << std::endl;

        svlFilterSourceVideoFile    *source = new svlFilterSourceVideoFile(1);
        svlFilterVideoFileWriter    *writer = new svlFilterVideoFileWriter;
        svlFilterImageCropper       *cropper = new svlFilterImageCropper;
        svlFilterImageChannelSwapper *rgb_swapper = new svlFilterImageChannelSwapper;
        svlFilterImageFileWriter    *imageWriter= new svlFilterImageFileWriter;
        svlStreamManager            *SaveStream = new svlStreamManager(8);

        bool saveImages = SaveAsBMPButton->isChecked();

        std::stringstream videofilename;

        if (source->SetFilePath(FileName) != SVL_OK) {
            CMN_LOG_CLASS_RUN_ERROR << std::endl << "Wrong file name... " << std::endl;
            return;
        }

        if ( saveImages ) {
            imageWriter->SetFilePath(saveParameters.Path().Data + saveParameters.Prefix().Data, "bmp");
            imageWriter->EnableTimestamps();
            imageWriter->Record();
            CMN_LOG_CLASS_RUN_VERBOSE << "`Start Recording`: setting up image writer with prefix: `" << videofilename.str() << "`" << std::endl;
        }
        else {
            svlVideoCodecBase* codec = svlVideoIO::GetCodec(".cvi");
            if (!codec) {
                CMN_LOG_CLASS_RUN_VERBOSE << "`StartRecording` error: failed to initialize `CVI` video codec" << std::endl;
                return;
            }
            svlVideoIO::Compression* compression = codec->GetCompression();
            if (!compression) {
                CMN_LOG_CLASS_RUN_VERBOSE << "`StartRecording` error: failed to get `CVI` video compression parameters" << std::endl;
                return;
            }
            compression->data[0] = 4; // compression level

            videofilename << std::setprecision(3) << std::fixed << saveParameters.Path().Data <<saveParameters.Prefix().Data <<"_"<< saveParameters.Start() << ".cvi";

            CMN_LOG_CLASS_RUN_VERBOSE << "`Start Recording`: setting up video file `" << videofilename.str() << "`" << std::endl;

            writer->SetFilePath(videofilename.str(), 0);
            writer->SetCodecParams(compression, 0);
            writer->OpenFile(0);

            svlVideoIO::ReleaseCompression(compression);
            svlVideoIO::ReleaseCodec(codec);
            writer->Record();
        }

        svlFilterOutput* output = 0;

        SaveStream->SetSourceFilter(source);
        output = source->GetOutput();

        //use the main window settings to crop the image.
        if (this->Cropper.IsEnabled()) {
            cropper->SetRectangle(Cropper.GetRectangle());
            output->Connect(cropper->GetInput());
            output = cropper->GetOutput();
        }
        if (this->SwapRGB_Button->isChecked()) {
            output->Connect(rgb_swapper->GetInput());
            output = rgb_swapper->GetOutput();
        }

        if ( saveImages )
            output->Connect(imageWriter->GetInput());
        else
            output->Connect(writer->GetInput());

        // initialize stream
        if (SaveStream->Initialize() != SVL_OK) {
            CMN_LOG_CLASS_RUN_ERROR << "Failed when converting: '" << FileName << "' to '" << videofilename.str()  << std::endl;
            return;
        }

        CMN_LOG_CLASS_RUN_VERBOSE << "Converting: '" << FileName << "' to '" <<  videofilename.str()  << std::endl;

        source->SetRange(source->GetPositionAtTime(saveParameters.Start()),source->GetPositionAtTime(saveParameters.End()));
        source->SetTargetFrequency(1000.0); // as fast as possible
        source->SetLoop(false);

        // start stream
        if (SaveStream->Play() != SVL_OK) {
            CMN_LOG_CLASS_RUN_ERROR << "Failed when converting: '" << FileName << "' to '" << videofilename<< std::endl;
            return;
        }

        do {
            std::cerr << " > Frames processed: " << source->GetFrameCounter() << "     \r";
        } while (SaveStream->IsRunning() && SaveStream->WaitForStop(0.5) == SVL_WAIT_TIMEOUT);

        CMN_LOG_CLASS_RUN_VERBOSE << " > Frames processed: " << source->GetFrameCounter() << "           " << std::endl;

        if (SaveStream->GetStreamStatus() < 0) {
            // Some error
            CMN_LOG_CLASS_RUN_ERROR << " -!- Error occured during conversion." << std::endl;
        }
        else {
            // Success
            CMN_LOG_CLASS_RUN_VERBOSE << " > Conversion done." << std::endl;
        }
        // CMN_LOG_CLASS_RUN_VERBOSE << "Converted: '" << FileName << "' to '" <<  videofilename.str()  << std::endl;

        SaveStream->Stop();
        // release pipeline
        CMN_LOG_CLASS_RUN_VERBOSE << "Releasing Pipeline" <<std::endl;
        SaveStream->Release();
        CMN_LOG_CLASS_RUN_VERBOSE << "Released Pipeline" <<std::endl;

        mtsManagerLocal *LCM = mtsManagerLocal::GetInstance();
        LCM->RemoveComponent(source);
        LCM->RemoveComponent(writer);
        LCM->RemoveComponent(cropper);
        LCM->RemoveComponent(imageWriter);
        LCM->RemoveComponent(rgb_swapper);

        delete SaveStream;
        delete writer;
        delete imageWriter;
        delete source;
        delete cropper;
        delete rgb_swapper;

    }
}


void sdpPlayerVideo::Quit(void)
{
    CMN_LOG_CLASS_RUN_DEBUG << "Quit" << std::endl;
    this->Kill();
}


void sdpPlayerVideo::QSlotPlayClicked(void)
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


void sdpPlayerVideo::QSlotSeekSliderMoved(int c)
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


void sdpPlayerVideo::QSlotSyncCheck(bool checked)
{
    Sync = checked;
}


void sdpPlayerVideo::QSlotStopClicked(void)
{
    mtsDouble now = Time;

    if (Sync) {
        StopRequest(now);
    } else {
        PlayUntilTime = now;
    }
}


void sdpPlayerVideo::LoadData(void)
{

    SetupPipeline();

    vctInt2 range;

    range[0] = 0;
    range[1] = Source.GetLength();

    PlayerDataInfo.DataStart() = Source.GetTimeAtPosition(range[0]);
    PlayerDataInfo.DataEnd() = Source.GetTimeAtPosition(range[1]);

    if (Time < PlayerDataInfo.DataStart()) {
        Time = PlayerDataInfo.DataStart();
    }

    if (Time > PlayerDataInfo.DataEnd()) {
        Time = PlayerDataInfo.DataEnd();
    }

    Source.SetLoop(true);

    //This is the standard.
    PlayUntilTime = PlayerDataInfo.DataEnd();

    UpdatePlayerInfo(PlayerDataInfo);
    UpdateLimits();
}


void sdpPlayerVideo::QSlotSetSaveStartClicked(void)
{
    ExWidget.SaveStartSpin->setValue(Time.Data);
}


void sdpPlayerVideo::QSlotSetSaveEndClicked(void)
{
    ExWidget.SaveEndSpin->setValue(Time.Data);
}


void sdpPlayerVideo::QSlotOpenFileClicked(void){


    QString fileName = QFileDialog::getOpenFileName(&Widget, tr("Select video file"),tr("./"),tr("Audio (*.cvi *.avi *.mpg *.mov *.mpeg)"));

    FileName = fileName.toStdString();

    if (fileName.isEmpty()) {
        CMN_LOG_CLASS_RUN_WARNING<<"File not selected, no data to load"<<std::endl;
        return;
    }

    BaseAccess.LoadData();

    BottomSpinBox->setValue(VideoHeight);
    RightSpinBox->setValue(VideoWidth);

}

void sdpPlayerVideo::UpdateLimits()
{
    ExWidget.TimeSlider->setRange((int)PlayerDataInfo.DataStart(), (int)PlayerDataInfo.DataEnd());

    ExWidget.TimeStartLabel->setText( QString::number(PlayerDataInfo.DataStart(),'f', 3));
    ExWidget.TimeEndLabel->setText( QString::number( PlayerDataInfo.DataEnd(),'f', 3));

    ExWidget.SaveStartSpin->setRange( PlayerDataInfo.DataStart(), PlayerDataInfo.DataEnd());
    ExWidget.SaveEndSpin->setRange( PlayerDataInfo.DataStart(), PlayerDataInfo.DataEnd());
    ExWidget.SaveEndSpin->setValue(PlayerDataInfo.DataEnd());

}


void sdpPlayerVideo::SetupPipeline(void)
{

    StreamManager.Release();
    Source.SetChannelCount(1);

    if (Source.SetFilePath(FileName) != SVL_OK) {
        CMN_LOG_CLASS_RUN_ERROR << std::endl << "Wrong file name... " << std::endl;
        return;
    }

    if (!TimestampOverlay) {

        TimestampOverlay = new svlOverlayTimestamp (0, true, VideoWidget, svlRect(4, 4, 134, 21),
                                                    15.0, svlRGB(255, 200, 200), svlRGB(32, 32, 32));
        Overlay.AddOverlay(*TimestampOverlay);

        // chain filters to pipeline
        StreamManager.SetSourceFilter(&Source); // chain filters to pipeline
        Source.GetOutput()->Connect(Cropper.GetInput());
        Cropper.GetOutput()->Connect(Overlay.GetInput());
        Overlay.GetOutput()->Connect(VideoWidget->GetInput());

    }

    unsigned int w;
    unsigned int h;
    svlVideoCodecBase * codec = svlVideoIO::GetCodec(FileName);
    double f;

    if (codec)
        codec->Open(FileName,w,h,f);
    codec->Close();

    VideoWidth  = w;
    VideoHeight = h;
    svlVideoIO::ReleaseCodec(codec);

    if (CropButton->isChecked()) {
        Cropper.SetRectangle(CropRect);
        //Cropper.SetRectangle(0,0,555,555);
    }
    else {
        Cropper.SetRectangle(0,0,w,h);
    }

    //StreamManager.Initialize();

    StreamManager.Play();
    Source.Pause();

    TopSpinBox->setRange(0,     Source.GetHeight());
    BottomSpinBox->setRange(0,  Source.GetHeight());
    RightSpinBox->setRange(0,   Source.GetWidth());
    LeftSpinBox->setRange(0,    Source.GetWidth());

}

void sdpPlayerVideo::SetSynced(bool isSynced) {

    ExWidget.SyncCheck->setChecked(isSynced);
    Sync = isSynced;

}


void sdpPlayerVideo::QSlotSetRangeClicked(void) {

    SaveParameters.Start() = ExWidget.SaveStartSpin->value();
    SaveParameters.End() = ExWidget.SaveEndSpin->value();

    UpdateSaveParams(SaveParameters);
}

void sdpPlayerVideo::QSlotCropButtonClicked(bool checked) {

    if (checked)
    {

        //! @todo check if these are logical.
        CropRect = svlRect(LeftSpinBox->value(),TopSpinBox->value(),RightSpinBox->value(),BottomSpinBox->value());

        CMN_LOG_CLASS_RUN_VERBOSE<<"Crop set to :"<<
                                   LeftSpinBox->value()<<","<<
                                   TopSpinBox->value()<<","<<
                                   RightSpinBox->value()<<","<<
                                   BottomSpinBox->value()<<std::endl;

        SetupPipeline();

        //CropButton->setText("CropEnabled");
    }
    else {

        SetupPipeline();
       // CropButton->setText("CropDisabled");
    }
}

bool sdpPlayerVideo::eventFilter( QObject *dist, QEvent *event )
{
    if( event->type() == QEvent::KeyPress )
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>( event );

        if(keyEvent->key() == Qt::Key_Right){
            SeekForwardOne();
            std::cout<<"Key_Right"<<std::endl;
            event->accept();
            return true;
        }
        else if(keyEvent->key() == Qt::Key_Left){
            SeekReverseOne();
            event->accept();
            std::cout<<"Key_Left"<<std::endl;
            return true;
        }
    }

    return false;
}

void sdpPlayerVideo::SeekForwardOne() {

    int range = Source.GetLength();
    int p = Source.GetPosition()+1;

    if (p >= range) {
        p = range-1;
    }
    double t = Source.GetTimeAtPosition(p);

    if (Sync) {
        SeekRequest(t);
    } else {
        State = SEEK;
        Time = t;
    }
    PlayUntilTime = PlayerDataInfo.DataEnd();

}

void sdpPlayerVideo::SeekReverseOne(){

    int p = Source.GetPosition()-1;

    if (p < 0) {
        p = 0;
    }
    double t = Source.GetTimeAtPosition(p);

    if (Sync) {
        SeekRequest(t);
    } else {
        State = SEEK;
        Time = t;
    }
    PlayUntilTime = PlayerDataInfo.DataEnd();

}
