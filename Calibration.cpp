//
// Created by Piotr Meller on 14.11.18.
//

#include "Calibration.h"
#include <opencv2/ccalib.hpp>



Calibration::Calibration(std::string filename)
{
	cv::FileStorage fs;
	fs.open(filename, cv::FileStorage::READ);
	fs["camera_matrix"] >> cameraMatrix;
	fs["distortion_coefficients"] >> distortionCoefficient;
	fs.release();
}


Calibration::~Calibration()
{
}

cv::Mat Calibration::getCameraMatrix()
{
	return cameraMatrix.clone();
}

cv::Mat Calibration::getDistortionCoefficient()
{
	return distortionCoefficient.clone();
}

cv::Mat Calibration::undistort(const cv::Mat & img)
{
	cv::Mat undistort;
	cv::undistort(img, undistort, cameraMatrix, distortionCoefficient);
	return undistort;
}
