/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*
  $Id: sawNoteRecorderQtComponent.cpp 3069 2011-10-18 14:25:14Z adeguet1 $

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
#include <QScrollArea>

#include <cisstMultiTask/mtsVector.h> // for mtsStdStringVec
#include <cisstMultiTask/mtsTaskManager.h>
#include <cisstMultiTask/mtsInterfaceProvided.h>
#include <cisstOSAbstraction/osaGetTime.h>

#include <sawOpenAL/sawNoteRecorderQtComponent.h>

CMN_IMPLEMENT_SERVICES(sawNoteRecorderQtComponent);

sawNoteRecorderQtComponent::sawNoteRecorderQtComponent(const std::string & name, double period):
    mtsComponent(name)
{

    startTimer(period * 1000);

    ErrorMessageDialog = new QErrorMessage();

    // create the user interface
    NoteRecorderWidget.setupUi(&Widget);

    // create container frame
    QFrame *frame = new QFrame(&Widget);
    frame->setLayout(&NotesVLayout);
    frame->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setWidget(frame);
    scrollArea->setWidgetResizable(true);   //this is very important.

    NoteRecorderWidget.MainVLayout->addWidget(scrollArea);

    TimeServer = mtsTaskManager::GetInstance()->GetTimeServer();

    MakeQTConnections();

    mtsInterfaceProvided* provided = this->AddInterfaceProvided("ProvidesNoteLogger");
    if (provided) {
        provided->AddCommandWrite(&sawNoteRecorderQtComponent::LogNote,  this, "LogNote", mtsStdString());
    }
}


void sawNoteRecorderQtComponent::MakeQTConnections(void)
{
    QObject::connect(NoteRecorderWidget.PathButton, SIGNAL(clicked(void)),
                     this, SLOT(QSlotPathClicked(void)));
    QObject::connect(NoteRecorderWidget.AddNoteButton, SIGNAL(clicked()),
                     this, SLOT(QSlotAddExtraNoteClicked()));
    QObject::connect(NoteRecorderWidget.LoadNotesButton, SIGNAL(clicked()),
                     this, SLOT(QSlotLoadPresetsClicked()));
    QObject::connect(NoteRecorderWidget.SaveNotesButton, SIGNAL(clicked()),
                     this, SLOT(QSlotSavePresetsClicked()));
    QObject::connect(NoteRecorderWidget.NewFileButton, SIGNAL(clicked()),
                     this, SLOT(QSlotNewLogFileClicked()));
}


void sawNoteRecorderQtComponent::Configure(const std::string & CMN_UNUSED(filename))
{

    //Widget.resize(700,300);
    AddNote(tr("sample note"));
    Widget.show();

}

void sawNoteRecorderQtComponent::ErrorMessage(const std::string & message)
{
    //    ErrorMessageDialog->showMessage(tr(msg.c_str()));
    QMessageBox::critical(this->GetWidget(), tr(GetName().c_str()), tr(message.c_str()));
}

sawNoteRecorderQtComponent::~sawNoteRecorderQtComponent() {
    LogFile.close();
}

void sawNoteRecorderQtComponent::QSlotPathClicked(void)
{
    QString pathName = QFileDialog::getExistingDirectory(&Widget, tr("Select Path"), tr("./"));
    NoteRecorderWidget.PathLineEdit->setText(pathName);
}

void sawNoteRecorderQtComponent::timerEvent(QTimerEvent * CMN_UNUSED(event))
{

    for (int i = 0; i < NoteVec.size() ; i++) {
        NoteVec[i]->UpdateGUI(TimeServer.GetAbsoluteTimeInSeconds());
    }
}

void sawNoteRecorderQtComponent::AddNote(const QString & noteTxt) {

    NoteWidget *note = new NoteWidget(this, noteTxt);
    NoteVec.push_front(note);
    NotesVLayout.addWidget(note);

}

void sawNoteRecorderQtComponent::QSlotRemoveNoteClicked(NoteWidget *note) {

    CMN_LOG_CLASS_RUN_VERBOSE << "Removing Note: " << note->GetText().toStdString() << std::endl;

    NotesVLayout.removeWidget(note);

    int i = NoteVec.indexOf(note);
    if (i != -1) {
        NoteVec.remove(i);
        delete note;
    }
    else
        CMN_LOG_CLASS_RUN_ERROR<< "" << i << std::endl;

}

void sawNoteRecorderQtComponent::QSlotLogNoteClicked(NoteWidget *note) {

    double time = TimeServer.GetAbsoluteTimeInSeconds();

    QString str(QString::number(time,'f', 3) + QString(':') + note->GetText());

    if (!LogFile.is_open()) {
        QSlotNewLogFileClicked();
    }
    CMN_LOG_CLASS_RUN_VERBOSE << "Saving Note: " << str.toStdString() << std::endl;

    LogFile<<str.toStdString()<<std::endl;

}


void sawNoteRecorderQtComponent::LogNote(const mtsStdString &note) {

    double time = TimeServer.GetAbsoluteTimeInSeconds();

    QString str(QString::number(time,'f', 3) + QString(':') + QString::fromStdString(note));

    if (!LogFile.is_open()) {
        QSlotNewLogFileClicked();
    }

    CMN_LOG_CLASS_RUN_VERBOSE << "Saving Note: " << str.toStdString() << std::endl;

    LogFile<<str.toStdString()<<std::endl;

}

void sawNoteRecorderQtComponent::QSlotAddExtraNoteClicked() {

    AddNote(tr(""));

}


void sawNoteRecorderQtComponent::LoadPresets() {

    CMN_LOG_CLASS_RUN_VERBOSE << "LoadNotes : " << std::endl;

    //open file dialog.
    QString fileName = QFileDialog::getOpenFileName(&Widget, tr("Select notes text file"), tr("./"), tr("Text (*.txt)"));

    std::ifstream notesFile(fileName.toStdString().c_str());

    if (notesFile.is_open())
    {

        //remove current ones.
        while (!NoteVec.empty()) {
            QSlotRemoveNoteClicked(NoteVec[0]);
        }

        std::string line;
        while ( notesFile.good() )
        {
            getline (notesFile,line);
            AddNote(QString::fromStdString(line));
        }

        notesFile.close();
        CMN_LOG_CLASS_RUN_VERBOSE<< "Loaded "<< NoteVec.size()<<" GUI notes deom "<< fileName.toStdString()<<std::endl;

    }
    else {
        CMN_LOG_CLASS_RUN_ERROR<< "Unable to open file" << fileName.toStdString()<<std::endl;
        ErrorMessage("Unable to open file" + fileName.toStdString());
    }

}

void sawNoteRecorderQtComponent::SavePresets() {

    CMN_LOG_CLASS_RUN_VERBOSE << "SaveNotes : " << std::endl;

    QString fileName = QFileDialog::getSaveFileName(&Widget, tr("Select save file"), tr("./"), tr("Text (*.txt)"));

    std::ofstream notesFile(fileName.toStdString().c_str());
    if (notesFile.is_open())
    {
        for (int i =  NoteVec.size() - 1; i > 0; i--) {
            notesFile<<NoteVec[i]->GetText().toStdString()<<std::endl;
        }
        notesFile<<NoteVec[0]->GetText().toStdString();  //avoid the last new line.

        notesFile.close();
        CMN_LOG_CLASS_RUN_VERBOSE<< "Saved "<< NoteVec.size()<<" GUI notes to "<< fileName.toStdString()<<std::endl;

    }
    else {
        CMN_LOG_CLASS_RUN_ERROR<< "Unable to open file" << fileName.toStdString()<<std::endl;
        ErrorMessage("Unable to open file" + fileName.toStdString());
    }

}


void sawNoteRecorderQtComponent::QSlotNewLogFileClicked()
{

    LogFile.close();
    std::string date;
    osaGetDateTimeString(date);

    std::string fileName = NoteRecorderWidget.PathLineEdit->text().toStdString() + "/" + NoteRecorderWidget.FileNameLineEdit->text().toStdString()
            +"_" + date + ".txt";


    LogFile.open(fileName.c_str());
    if (LogFile.is_open())
    {
        CMN_LOG_CLASS_RUN_VERBOSE<< "Opened logfile (" << fileName << ") file for writing"<<std::endl;

    }
    else {
        CMN_LOG_CLASS_RUN_ERROR<< "Unable to open file" << fileName<<std::endl;
        ErrorMessage("Unable to open file" + fileName);
    }

}

