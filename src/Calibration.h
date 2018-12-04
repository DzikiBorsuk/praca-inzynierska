//
// Created by Piotr Meller on 14.11.18.
//

#ifndef PRACA_INZYNIERSKA_CALIBRATION_H
#define PRACA_INZYNIERSKA_CALIBRATION_H

#include <opencv2/core.hpp>

class Calibration
{
    cv::Mat cameraMatrix;
    cv::Mat distortionCoefficient;
    std::vector<cv::Mat> imagesArray;
    std::vector<std::vector<cv::Point2f>> cornersArray;
    std::vector<std::vector<cv::Point3f>> realCornersArray;
    std::vector<cv::Mat> rvecArray;
    std::vector<cv::Mat> tvecArray;


public:

    enum PatternType
    {
        CHESSBOARD,
        CIRCLES_GRID,
        CIRCLES_GRID_ASYMMETRIC
    };

    Calibration(std::string filename);
    ~Calibration();

    cv::Mat getCameraMatrix();
    cv::Mat getDistortionCoefficient();

    void loadImagesList(std::vector<std::string> image_path_list);

    void calibrateCamera(cv::Size patern_size, int patternType = PatternType::CHESSBOARD);

    cv::Mat undistort(const cv::Mat &img);
};

#endif //PRACA_INZYNIERSKA_CALIBRATION_H

