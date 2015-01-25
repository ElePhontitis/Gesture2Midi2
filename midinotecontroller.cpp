#include "midinotecontroller.h"
#include <QDebug>

MidiNoteController::MidiNoteController()
{
}


void MidiNoteController::initPreset(){
    this->preset.clear();
    //First
        vector<int> firstChord;
        firstChord.push_back(0);
        firstChord.push_back(4);
        firstChord.push_back(7);

        //Second
        vector<int> secondChord;
        secondChord.push_back(9);
        secondChord.push_back(4);
        secondChord.push_back(7);

        vector<int> thirdChord;
        thirdChord.push_back(12);
        thirdChord.push_back(9);
        thirdChord.push_back(7);

        vector<int> fourthChord;
        fourthChord.push_back(12);
        fourthChord.push_back(4);
        fourthChord.push_back(9);

        vector<int> fifthChord;
        fifthChord.push_back(12);
        fifthChord.push_back(16);
        fifthChord.push_back(7);

        this->preset.push_back(firstChord);
        this->preset.push_back(secondChord);
        this->preset.push_back(thirdChord);
        this->preset.push_back(fourthChord);
        this->preset.push_back(fifthChord);
}

void MidiNoteController::initDurianPreset(){
    this->preset.clear();
    //First
        vector<int> firstChord;
        firstChord.push_back(0);
        firstChord.push_back(3);
        firstChord.push_back(7);

        //Second
        vector<int> secondChord;
        secondChord.push_back(3);
        secondChord.push_back(10);
        secondChord.push_back(7);

        vector<int> thirdChord;
        thirdChord.push_back(12);
        thirdChord.push_back(3);
        thirdChord.push_back(10);

        vector<int> fourthChord;
        fourthChord.push_back(10);
        fourthChord.push_back(13);
        fourthChord.push_back(17);

        vector<int> fifthChord;
        fifthChord.push_back(10);
        fifthChord.push_back(3);
        fifthChord.push_back(6);

        this->preset.push_back(firstChord);
        this->preset.push_back(secondChord);
        this->preset.push_back(thirdChord);
        this->preset.push_back(fourthChord);
        this->preset.push_back(fifthChord);
}

void MidiNoteController::initOverdurPreset(){
    this->preset.clear();
    //First
        vector<int> firstChord;
        firstChord.push_back(0);
        firstChord.push_back(4);
        firstChord.push_back(8);

        //Second
        vector<int> secondChord;
        secondChord.push_back(8);
        secondChord.push_back(4);
        secondChord.push_back(12);

        vector<int> thirdChord;
        thirdChord.push_back(0);
        thirdChord.push_back(4);
        thirdChord.push_back(10);

        vector<int> fourthChord;
        fourthChord.push_back(8);
        fourthChord.push_back(10);
        fourthChord.push_back(15);

        vector<int> fifthChord;
        fifthChord.push_back(0);
        fifthChord.push_back(8);
        fifthChord.push_back(5);

        this->preset.push_back(firstChord);
        this->preset.push_back(secondChord);
        this->preset.push_back(thirdChord);
        this->preset.push_back(fourthChord);
        this->preset.push_back(fifthChord);
}

map<string, int> MidiNoteController::midiMap(){
    map<string,int> midiMap;
     midiMap["C"] = 12;
     midiMap["C#"] = 13;
     midiMap["D"] = 14;
     midiMap["D#"] = 15;
     midiMap["E"] = 16;
     midiMap["F"] = 17;
     midiMap["F#"] = 18;
     midiMap["G"] = 19;
     midiMap["G#"] = 20;
     midiMap["A"] = 21;
     midiMap["A#"] = 22;
     midiMap["B"] = 23;
       return midiMap;
}

void MidiNoteController::setCurrentPreset(vector<vector<int> > preset){
        this->preset = preset;
}

void MidiNoteController::setPresetWithName(string presetName){
    if (presetName.compare("Molly") == 1){
        this->initPreset();
    } else if (presetName.compare("Durian") == 1){
        this->initDurianPreset();
    } else if (presetName.compare("Overdur") == 1){
        this->initOverdurPreset();
    }
}

void MidiNoteController::setNoteForNoteWithOctave(string note, int octave){
        map<string,int> midiMap= this->midiMap();
        int noteValue = midiMap[note];
        this->currentNote =  (noteValue + (12 * octave));
        //qDebug() << "New grundton " << this->currentNote;
}

vector<int> MidiNoteController::getCurrentNotes(int numberOfFingers){
        qDebug() << "Finger: " << numberOfFingers;
        vector<int> chord (this->preset.at(numberOfFingers));
        vector<int> currentNotes;
        for (int note : chord){
            int resultNote = note + this->currentNote;
            currentNotes.push_back(resultNote);
        }
        return currentNotes;
}
