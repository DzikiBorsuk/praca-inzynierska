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
    cv::Size patternSize;
    std::vector<cv::Mat> imagesArray;
    std::vector<cv::Mat> imagesArrayGray;
    std::vector<cv::Mat> undistortedImagesArray;
    std::vector<bool> goodImages;
    std::vector<std::vector<cv::Point2f>> cornersArray;
    std::vector<std::vector<cv::Point3f>> realCornersArray;
    std::vector<cv::Mat> rvecArray;
    std::vector<cv::Mat> tvecArray;
    std::vector<double> reprojectionErrorsArray;
    double avgError;
    double avgError2;


public:

    enum PatternType
    {
        CHESSBOARD,
        CIRCLES_GRID,
        CIRCLES_GRID_ASYMMETRIC
    };

    enum CalibOptions
    {

    };

    Calibration() = default;
    Calibration(std::string filename);
    ~Calibration();

    void loadParams(const std::string &filename);
    void saveParams(const std::string &filename);

    cv::Mat getCameraMatrix();
    cv::Mat getDistortionCoefficient();

    void loadImagesList(const std::vector<std::string> &image_path_list);

    void findCalibrationPoints(const cv::Size &pattern_size,
                               int patternType = PatternType::CHESSBOARD,
                               float squareSize = 1,
                               cv::InputArray mask = cv::noArray());

    void calibrateCamera(int flags = 0);

    void calculateReprojectionError();

    void undistortCalibrationImages();

    void drawPattern();

    cv::Mat undistort(const cv::Mat &img);
    const std::vector<cv::Mat> &getImagesArray() const;
    const std::vector<bool> &getGoodImages() const;
    const std::vector<double> &getReprojectionErrorsArray() const;
    const cv::Mat &getImage(int i) const;
    const cv::Mat &getUndistortedImage(int i) const;

    double getAvgError2() const;
};

#endif //PRACA_INZYNIERSKA_CALIBRATION_H

