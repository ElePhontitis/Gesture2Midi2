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
    ui->midiControllerValueDisplay->setPalette(Qt::black);
    QStringList connections = midiOutput.connections(true);
    ui->comboBox->addItems(connections);

    connect(spacebar, SIGNAL(activated()), this, SLOT(toggleButtonBySpaceBar()));
    midiOutput.open("LoopBe Internal MIDI");
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
        this->ui->midiControllerValueDisplay->setDigitCount(midiControllerValue);
        qDebug()<<"Midi-Controller-Out: "<<midiControllerValue;
    if (this->colorKeyerHSV->handAnalyzer->isSchlag()){
        qDebug() << "Schlag im MainWindow";

        int numberOfFingers = this->colorKeyerHSV->handAnalyzer->getNumberOfFingers();
        vector<int> currentNotes = this->colorKeyerHSV->handAnalyzer->midiNoteController->getCurrentNotes(numberOfFingers);
        vector<int> notesToTurnOff;
        vector<int> notesToTurnOn;

        if (!this->currentlyPlaying.empty()){
            qDebug() << "Not empty you snob! no. of el: " << this->currentlyPlaying.size();
            //Check which notes needs to be turned off etc.
           vector<int> intersection =  this->instersection(this->currentlyPlaying, currentNotes);
           notesToTurnOff = this->difference(intersection, this->currentlyPlaying);
           notesToTurnOn = this->difference(intersection, currentNotes);
        } else {
            for (int it : currentNotes){
                notesToTurnOn.push_back(it);
            }
        }


    qDebug() << "Number of notes to turn on given by the midinoteController " << currentNotes.size();
    qDebug() << "Number off notes to turn on: " << notesToTurnOn.size();

       for (int i = 0; i < notesToTurnOff.size(); i++){
           qDebug() << "Turn off note: " << notesToTurnOff.at(i);
           this->midiOutput.sendNoteOff(midichannel, notesToTurnOff.at(i), 0);
       }

        for (int i = 0; i < notesToTurnOn.size(); i++){
            qDebug() << "Turn on note: " << notesToTurnOn.at(i);
            this->midiOutput.sendNoteOn(midichannel, notesToTurnOn.at(i), 127);
        }
        this->currentlyPlaying.clear();
        for (int i = 0; i < currentNotes.size(); i++){
            this->currentlyPlaying.push_back(currentNotes.at(i));
        }

        this->ui->midiControllerValueDisplay->display(midiControllerValue);
        this->ui->midiControllerValueKnob->setValue(midiControllerValue);

    }
}
vector<int> MainWindow::instersection(vector<int> v1, vector<int> v2)
{

    vector<int> v3;

    sort(v1.begin(), v1.end());
    sort(v2.begin(), v2.end());

    set_intersection(v1.begin(),v1.end(),v2.begin(),v2.end(),back_inserter(v3));

     qDebug() <<"Intersection";
    for (int i : v3){
        qDebug() << "returne: " << i;
    }

    return v3;
}

vector<int> MainWindow::difference(vector<int> v1, vector<int> v2)
{

    vector<int> v3;

    sort(v1.begin(), v1.end());
    sort(v2.begin(), v2.end());

    set_difference(v1.begin(),v1.end(),v2.begin(),v2.end(),back_inserter(v3));

    qDebug() <<"Differenz";
    for (int i : v3){
        qDebug() << "returne: " << i;
    }

    return v3;
}


void MainWindow::on_sendMidiControllerCheckbox_toggled(bool checked){
     this->ui->midiMappingModeCheckBox->setDisabled(checked);
     this->ui->midiMappingModeButton->setDisabled(checked);
    if(checked){
        connect(&timer, SIGNAL(timeout()),this, SLOT(sendMidiParameter()));
        timer.start(50);
    }else{
        timer.stop();
        if (this->isPlaying){
            for (int i = 0; i < this->currentlyPlaying.size(); i++){
                this->midiOutput.sendNoteOff(midichannel, this->currentlyPlaying.at(i), 0);
            }
            this->currentlyPlaying.clear();
            //for(vector<int>::iterator it = this->currentlyPlaying.begin(); it != this->currentlyPlaying.end(); ++it) {
              //  this->midiOutput.sendNoteOff(midichannel, *it, 0);
            //}
            this->isPlaying = false;
        }
    }
}

void MainWindow::on_midiMappingModeCheckBox_toggled(bool checked){
     this->ui->sendMidiControllerCheckbox->setDisabled(checked);
    if (checked) {
        timer.stop();


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

void MainWindow::on_grundTonComboBox_currentIndexChanged(const QString &arg1)
{
    if (this->isPlaying){
        for (int i = 0; i < this->currentlyPlaying.size(); i++){
            this->midiOutput.sendNoteOff(midichannel, this->currentlyPlaying.at(i), 0);
        }
        this->currentlyPlaying.clear();
        //for(vector<int>::iterator it = this->currentlyPlaying.begin(); it != this->currentlyPlaying.end(); ++it) {
          //  this->midiOutput.sendNoteOff(midichannel, *it, 0);
        //}
        this->isPlaying = false;
    }
    string castedArg1 = arg1.toStdString();
    int oktave = this->ui->oktavenComboBox->currentText().toInt();
    this->colorKeyerHSV->handAnalyzer->midiNoteController->setNoteForNoteWithOctave(castedArg1, oktave);
}


void MainWindow::on_oktavenComboBox_activated(const QString &arg1)
{
    if (this->isPlaying){
        for (int i = 0; i < this->currentlyPlaying.size(); i++){
            this->midiOutput.sendNoteOff(midichannel, this->currentlyPlaying.at(i), 0);
        }
        this->currentlyPlaying.clear();
        //for(vector<int>::iterator it = this->currentlyPlaying.begin(); it != this->currentlyPlaying.end(); ++it) {
          //  this->midiOutput.sendNoteOff(midichannel, *it, 0);
        //}
        this->isPlaying = false;
    }
    string grundton = this->ui->grundTonComboBox->currentText().toStdString();
    int oktave = arg1.toInt();
    this->colorKeyerHSV->handAnalyzer->midiNoteController->setNoteForNoteWithOctave(grundton, oktave);
}
