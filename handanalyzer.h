#ifndef HANDANALYZER_H
#define HANDANALYZER_H
#include <opencv2/opencv.hpp>
#include "midiparametercontroller.h"
#include "midinotecontroller.h"
class HandAnalyzer
{
public:
    HandAnalyzer();
    cv::Mat getResultMatFromMat(const cv::Mat&source);
    bool isSchlag();
    void defineFingerDepth(cv::vector<cv::Point> cnt, const cv::Mat &input);
    void setNumberOfFinger(int n);
    void setRectAverage(int rectA);
    int getNumberOfFingers();
    MidiParameterController *midiParameterController;
    MidiNoteController *midiNoteController;

private:
    int numberOfFingers;
    float depthThreshold;
    int rectAverages[8];
    int fingerValues[6];
    bool schlag;
};

#endif // HANDANALYZER_H
