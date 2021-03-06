#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <opencv2/opencv.hpp>
#include "videoengine.h"
#include "colorkeyerhsv.h"
#include "QShortcut"

using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    colorKeyerHSV(new ColorKeyerHSV())
{

    QShortcut *spacebar = new QShortcut(QKeySequence(Qt::Key_Space),this);
    ui->setupUi(this);
    ui->midiControllerSpinBox->setValue(16);
    connect(spacebar, SIGNAL(activated()), this, SLOT(toggleButtonBySpaceBar()));
    ui->midiControllerValueDisplay->setPalette(Qt::green);
    QStringList connections = midiOutput.connections(true);
    ui->comboBox->addItems(connections);
    connect(&controllerTimer, SIGNAL(timeout()),this, SLOT(sendMidiParameter()));
    connect(&noteTimer, SIGNAL(timeout()),this, SLOT(sendMidiNotes()));
    connect(spacebar, SIGNAL(activated()), this, SLOT(toggleButtonBySpaceBar()));
    midiOutput.open("Gesture2Midi");
    ui->comboBox->setCurrentText("Gesture2Midi");
    midichannel = ui->midichannel->value();
    colorKeyerHSV->setHueTolerance(ui->hueTolerance->value());
    colorKeyerHSV->setHueValue(ui->hueValue->value());
    colorKeyerHSV->setSaturationTolerance(ui->saturationTolerance->value());
    colorKeyerHSV->setSaturationValue(ui->saturationValue->value());
    videoEngine = new VideoEngine();
    videoEngine -> setProcessor(colorKeyerHSV);
    connect(videoEngine, SIGNAL(sendProcessedImage(const QImage&)), ui->videoLabel, SLOT(setImage(const QImage&)));
    videoEngine -> openCamera();

    videoEngine -> start();
}

MainWindow::~MainWindow()

{
    midiOutput.close();
    delete colorKeyerHSV;
    delete ui;
    delete videoEngine;

}



void MainWindow::on_midichannel_valueChanged(int arg1)
{
    midichannel = arg1;
}

void MainWindow::on_comboBox_activated(const QString &arg1)
{
    midiOutput.open(arg1);
}



void MainWindow::on_checkBoxMedian_toggled(bool checked)
{
    colorKeyerHSV->setMedianEnable(checked);
}

void MainWindow::on_checkBoxUseOpen_toggled(bool checked)
{
    colorKeyerHSV->setOpeningEnable(checked);
}

void MainWindow::on_checkBoxMaskSmallRegions_toggled(bool checked)
{
    colorKeyerHSV->setMaskSmallRegions(checked);
}

void MainWindow::on_hueTolerance_valueChanged(int value)
{
    colorKeyerHSV->setHueTolerance(value);
}

void MainWindow::on_saturationTolerance_valueChanged(int value)
{
    colorKeyerHSV->setSaturationTolerance(value);
}

void MainWindow::on_hueValue_valueChanged(int value)
{
    colorKeyerHSV->setHueValue(value);
}

void MainWindow::on_saturationValue_valueChanged(int value)
{
    colorKeyerHSV->setSaturationValue(value);
}

void MainWindow::sendMidiParameter(){
        int midiControllerValue = colorKeyerHSV->handAnalyzer->midiParameterController->getMidiController();
        midiOutput.sendController(midichannel,midiControllerNumber,midiControllerValue);
        this->ui->midiControllerValueDisplay->display(midiControllerValue);
        this->ui->midiControllerValueKnob->setValue(midiControllerValue);



}
void MainWindow::sendMidiNotes(){


    if (this->colorKeyerHSV->handAnalyzer->isSchlag()){

        int numberOfFingers = this->colorKeyerHSV->handAnalyzer->getNumberOfFingers();
        vector<int> currentNotes = this->colorKeyerHSV->handAnalyzer->midiNoteController->getCurrentNotes(numberOfFingers);
        vector<int> notesToTurnOff;
        vector<int> notesToTurnOn;

        if (!this->currentlyPlaying.empty()){
            //Check which notes needs to be turned off etc.
           vector<int> intersection =  this->instersection(this->currentlyPlaying, currentNotes);
           notesToTurnOff = this->difference( this->currentlyPlaying, intersection);
           notesToTurnOn = this->difference(currentNotes, intersection);
        } else {
            for (int it : currentNotes){
                notesToTurnOn.push_back(it);
            }
        }


  //  qDebug() << "Number of notes to turn on given by the midinoteController " << currentNotes.size();
  //  qDebug() << "Number off notes to turn on: " << notesToTurnOn.size();

       for (int i = 0; i < notesToTurnOff.size(); i++){
        //   qDebug() << "Turn off note: " << notesToTurnOff.at(i);
           this->midiOutput.sendNoteOff(midichannel, notesToTurnOff.at(i), 0);
       }

        for (int i = 0; i < notesToTurnOn.size(); i++){
          //  qDebug() << "Turn on note: " << notesToTurnOn.at(i);
            this->midiOutput.sendNoteOn(midichannel, notesToTurnOn.at(i), 127);
        }
        this->currentlyPlaying.clear();
        for (int i = 0; i < currentNotes.size(); i++){
            this->currentlyPlaying.push_back(currentNotes.at(i));
        }



    }
}
vector<int> MainWindow::instersection(vector<int> v1, vector<int> v2)
{

    vector<int> v3;

    sort(v1.begin(), v1.end());
    sort(v2.begin(), v2.end());

    set_intersection(v1.begin(),v1.end(),v2.begin(),v2.end(),back_inserter(v3));

    return v3;
}

vector<int> MainWindow::difference(vector<int> v1, vector<int> v2)
{

    vector<int> v3;

    sort(v1.begin(), v1.end());
    sort(v2.begin(), v2.end());

    set_difference(v1.begin(),v1.end(),v2.begin(),v2.end(),back_inserter(v3));

    return v3;
}


void MainWindow::on_sendMidiControllerCheckbox_toggled(bool checked){
     this->ui->midiMappingModeCheckBox->setDisabled(checked);
     this->ui->midiMappingModeButton->setDisabled(checked);
    if(checked){
        noteTimer.start(51);
        controllerTimer.start(47);
    }else{
        noteTimer.stop();
        controllerTimer.stop();
            for (int i = 0; i < this->currentlyPlaying.size(); i++){
                this->midiOutput.sendNoteOff(midichannel, this->currentlyPlaying.at(i), 0);
            }
            this->currentlyPlaying.clear();
    }
}

void MainWindow::on_midiMappingModeCheckBox_toggled(bool checked){
     this->ui->sendMidiControllerCheckbox->setDisabled(checked);
    if (checked) {
        noteTimer.stop();
        controllerTimer.stop();
        for (int i = 0; i < this->currentlyPlaying.size(); i++){
            this->midiOutput.sendNoteOff(midichannel, this->currentlyPlaying.at(i), 0);
        }
        this->currentlyPlaying.clear();

    }
}

void MainWindow::on_midiMappingModeButton_pressed()
{
    midiOutput.sendController(midichannel,midiControllerNumber,127);
}

void MainWindow::on_midiMappingModeButton_released()
{
    midiOutput.sendController(midichannel,midiControllerNumber,0);
}
void MainWindow::toggleButtonBySpaceBar()
{
    this->ui->midiMappingModeButton->animateClick();
}

void MainWindow::on_midiControllerSpinBox_valueChanged(int arg1)
{
    midiControllerNumber = arg1;
}

void MainWindow::setUpCurrentNote(){
    //Stop all playing notes at first
        for (int i = 0; i < this->currentlyPlaying.size(); i++){
            this->midiOutput.sendNoteOff(midichannel, this->currentlyPlaying.at(i), 0);
        }
        this->currentlyPlaying.clear();

    //Then set the new note
    string grundton = this->ui->rootComboBox->currentText().toStdString();
    int oktave = this->ui->octaveSpinBox->value();
    this->colorKeyerHSV->handAnalyzer->midiNoteController->setNoteForNoteWithOctave(grundton, oktave);
}

void MainWindow::on_rootComboBox_activated(const QString &arg1)
{
    this->setUpCurrentNote();
}

void MainWindow::on_octaveSpinBox_valueChanged(int arg1)
{
    this->setUpCurrentNote();
}

void MainWindow::on_presetComboBox_activated(const QString &arg1)
{
        for (int i = 0; i < this->currentlyPlaying.size(); i++){
            this->midiOutput.sendNoteOff(midichannel, this->currentlyPlaying.at(i), 0);
        }
        this->currentlyPlaying.clear();

    this->colorKeyerHSV->handAnalyzer->midiNoteController->setPresetWithName(arg1.toStdString());
}
