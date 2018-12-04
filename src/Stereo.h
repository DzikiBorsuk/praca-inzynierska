//
// Created by Piotr Meller on 20.10.18.
//

#ifndef PRACA_INZYNIERSKA_STEREO_H
#define PRACA_INZYNIERSKA_STEREO_H


//#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include "Rectification.h"
#include "Calibration.h"
#include "Disparity.h"


class Stereo
{
public:

    Calibration calib;
    Rectification rect;
    Disparity disp;

private:
    cv::Mat left, middle, right;
    //cv::Mat left_disp,right_disp,filtered_disp;
    //bool run_calibration;

    std::vector<cv::KeyPoint> keypoints_left, keypoints_right;
    std::vector<cv::Point2f> lk, rk;

    enum class imgNum
    {
        two = 2, three = 3,
    };

    imgNum numberOfImage;


public:
    //Stereo(const std::string[] img, const std::string &_middle, const std::string &_right, const std::string &_cameraParamsFile="");
    Stereo(const std::string &_left, const std::string &_right, const std::string &_cameraParamsFile = "");

    void rectifyImage();

    void computeDisp();

    void show();

//private:
    void match_feautures();
};


#endif //PRACA_INZYNIERSKA_STEREO_H
