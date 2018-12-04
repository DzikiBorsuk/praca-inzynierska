//
// Created by Piotr Meller on 14.11.18.
//

#include "Calibration.h"
#include <opencv2/ccalib.hpp>
#include <opencv2/highgui.hpp>

Calibration::Calibration(std::string filename)
{
    cv::FileStorage fs;
    fs.open(filename, cv::FileStorage::READ);
    fs["camera_matrix"] >> cameraMatrix;
    fs["distortion_coefficients"] >> distortionCoefficient;
    fs.release();
}

Calibration::~Calibration() = default;

cv::Mat Calibration::getCameraMatrix()
{
    return cameraMatrix.clone();
}

cv::Mat Calibration::getDistortionCoefficient()
{
    return distortionCoefficient.clone();
}

void Calibration::calibrateCamera(std::vector<std::string> image_path_list, cv::Size patern_size)
{

    cv::Mat image;
    for (int i = 0; i < image_path_list.size(); ++i)
    {
        image =
    }

}

cv::Mat Calibration::undistort(const cv::Mat &img)
{
    cv::Mat undistort;
    cv::undistort(img, undistort, cameraMatrix, distortionCoefficient);
    return undistort;
}
