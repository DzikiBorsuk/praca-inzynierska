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

void Calibration::loadImagesList(std::vector<std::string> image_path_list)
{
    imagesArray.clear();
    imagesArray.resize(image_path_list.size());
    for (int i = 0; i < image_path_list.size(); ++i)
    {
        imagesArray[i] = cv::imread(image_path_list[i]);
    }
}

void Calibration::calibrateCamera(cv::Size patern_size, int patternType)
{
    cornersArray.resize(imagesArray.size());

    for (int i = 0; i < imagesArray.size(); ++i)
    {

        if (patternType == PatternType::CHESSBOARD)
        {
            cv::findChessboardCorners(imagesArray[i], patern_size, cornersArray[i]);
        }
        else if (patternType == PatternType::CIRCLES_GRID)
        {
            cv::findCirclesGrid(imagesArray[i], patern_size, cornersArray[i], cv::CALIB_CB_SYMMETRIC_GRID);
        }
        else if (patternType == PatternType::CIRCLES_GRID_ASYMMETRIC)
        {
            cv::findCirclesGrid(imagesArray[i], patern_size, cornersArray[i], cv::CALIB_CB_ASYMMETRIC_GRID);
        }
    }

    realCornersArray.clear();
    realCornersArray.emplace_back();
    realCornersArray[0].reserve(static_cast<unsigned long>(patern_size.height * patern_size.width));

    int squareSize = 1;

    switch (patternType)
    {
        case PatternType::CHESSBOARD:
        case PatternType::CIRCLES_GRID:
            for (int i = 0; i < patern_size.height; ++i)
            {
                for (int j = 0; j < patern_size.width; ++j)
                {
                    realCornersArray[0].push_back(cv::Point3f(j * squareSize, i * squareSize, 0));
                }
            }
            break;

        case PatternType::CIRCLES_GRID_ASYMMETRIC:
            for (int i = 0; i < patern_size.height; i++)
            {
                for (int j = 0; j < patern_size.width; j++)
                {
                    realCornersArray[0].push_back(cv::Point3f((2 * j + i % 2) * squareSize, i * squareSize, 0));
                }
            }
            break;
        default:break;
    }

    realCornersArray.resize(imagesArray.size(), realCornersArray[0]);

    cv::calibrateCamera(realCornersArray,
                        cornersArray,
                        imagesArray[0].size(),
                        cameraMatrix,
                        distortionCoefficient,
                        rvecArray,
                        tvecArray, cv::CALIB_ZERO_TANGENT_DIST);

    
}

cv::Mat Calibration::undistort(const cv::Mat &img)
{
    cv::Mat undistort;
    cv::undistort(img, undistort, cameraMatrix, distortionCoefficient);
    return undistort;
}

