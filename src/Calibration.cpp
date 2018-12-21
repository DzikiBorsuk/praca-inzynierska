//
// Created by Piotr Meller on 14.11.18.
//

#include "Calibration.h"
#include <opencv2/ccalib.hpp>
#include <opencv2/highgui.hpp>

Calibration::Calibration()
{
	cv::FileStorage fs;
    fs.open("deafault_camera_params.xml", cv::FileStorage::READ);
    if (!fs.isOpened())
	{
        fs.open("deafault_camera_params.yml", cv::FileStorage::READ);
        if (!fs.isOpened())
		{
			return;
		}
	}

	fs["camera_matrix"] >> cameraMatrix;
	fs["distortion_coefficients"] >> distortionCoefficient;
	fs.release();
}

Calibration::Calibration(std::string filename)
{
    loadParams(filename);
}

Calibration::~Calibration() = default;

void Calibration::loadParams(const std::string &filename)
{
    cv::FileStorage fs;
    fs.open(filename, cv::FileStorage::READ);
    fs["camera_matrix"] >> cameraMatrix;
    fs["distortion_coefficients"] >> distortionCoefficient;
    fs.release();
}

void Calibration::saveParams(const std::string &filename)
{
    cv::FileStorage fs(filename, cv::FileStorage::WRITE);

    fs << "camera_matrix" << cameraMatrix;
    fs << "distortion_coefficients" << distortionCoefficient;
    fs << "pattern_size" << patternSize;
    fs << "corners" << cornersArray;
    fs << "rvec_array" << rvecArray;
    fs << "tvec_array" << tvecArray;

    fs.release();

}

cv::Mat Calibration::getCameraMatrix()
{
    return cameraMatrix.clone();
}

cv::Mat Calibration::getDistortionCoefficient()
{
    return distortionCoefficient.clone();
}

void Calibration::loadImagesList(const std::vector<std::string> &image_path_list)
{
    imagesArray.clear();
    imagesArray.resize(image_path_list.size());
    imagesArrayGray.clear();
    imagesArrayGray.resize(image_path_list.size());
    for (int i = 0; i < image_path_list.size(); ++i)
    {
        imagesArray[i] = cv::imread(image_path_list[i]);
        cv::cvtColor(imagesArray[i], imagesArrayGray[i], cv::COLOR_BGR2GRAY);
    }
}

void Calibration::findCalibrationPoints(const cv::Size &pattern_size,
                                        int patternType,
                                        float squareSize,
                                        cv::InputArray mask)
{
    cornersArray.clear();
    cornersArray.resize(imagesArray.size());
    patternSize = pattern_size;

    goodImages.clear();
    goodImages.reserve(imagesArray.size());
    bool cornerFound = false;

    for (int i = 0; i < imagesArray.size(); ++i)
    {

        if (patternType == PatternType::CHESSBOARD)
        {
            cornerFound = cv::findChessboardCorners(imagesArray[i], pattern_size, cornersArray[i]);
        }
        else if (patternType == PatternType::CIRCLES_GRID)
        {
            cornerFound =
                cv::findCirclesGrid(imagesArray[i], pattern_size, cornersArray[i], cv::CALIB_CB_SYMMETRIC_GRID);
        }
        else if (patternType == PatternType::CIRCLES_GRID_ASYMMETRIC)
        {
            cornerFound =
                cv::findCirclesGrid(imagesArray[i], pattern_size, cornersArray[i], cv::CALIB_CB_ASYMMETRIC_GRID);
        }

        //cv::find4QuadCornerSubpix(imagesArrayGray[i],cornersArray[i],pattern_size);
        if(cornerFound)
        {
        cv::cornerSubPix(imagesArrayGray[i],
                         cornersArray[i],
                         cv::Size(21, 21),
                         cv::Size(-1, -1),
                         cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 120, 0.01));
        }

        goodImages.push_back(cornerFound);
    }

    realCornersArray.clear();
    realCornersArray.emplace_back();
    realCornersArray[0].reserve(static_cast<unsigned long>(pattern_size.height * pattern_size.width));


    switch (patternType)
    {
        case PatternType::CHESSBOARD:
        case PatternType::CIRCLES_GRID:
            for (int i = 0; i < pattern_size.height; ++i)
            {
                for (int j = 0; j < pattern_size.width; ++j)
                {
                    realCornersArray[0].push_back(cv::Point3f(j * squareSize, i * squareSize, 0));
                }
            }
            break;

        case PatternType::CIRCLES_GRID_ASYMMETRIC:
            for (int i = 0; i < pattern_size.height; i++)
            {
                for (int j = 0; j < pattern_size.width; j++)
                {
                    realCornersArray[0].push_back(cv::Point3f((2 * j + i % 2) * squareSize, i * squareSize, 0));
                }
            }
            break;
        default:break;
    }

    realCornersArray.resize(imagesArray.size(), realCornersArray[0]);

}

void Calibration::calibrateCamera(int flags)
{
    std::vector<std::vector<cv::Point2f>> foundCorners;
    std::vector<std::vector<cv::Point3f>> foundRealCorners;
    foundCorners.reserve(cornersArray.size());
    foundRealCorners.reserve(cornersArray.size());
    for(int i=0;i<cornersArray.size();++i)
    {
        if(goodImages[i])
        {
            foundCorners.push_back(cornersArray[i]);
            foundRealCorners.push_back(realCornersArray[i]);
        }
    }

    avgError = cv::calibrateCamera(foundRealCorners,
                                   foundCorners,
                                   imagesArray[0].size(),
                                   cameraMatrix,
                                   distortionCoefficient,
                                   rvecArray,
                                   tvecArray, cv::CALIB_ZERO_TANGENT_DIST);
}

void Calibration::calculateReprojectionError()
{
    std::vector<cv::Point2f> projectionCorners;
    size_t totalPoints = 0;
    double totalErr = 0, err;

    reprojectionErrorsArray.clear();
    reprojectionErrorsArray.resize(imagesArray.size());
    //perViewErrors.resize(objectPoints.size());

    int j=0;

    for (size_t i = 0; i < reprojectionErrorsArray.size(); ++i)
    {
        if (goodImages[i])
        {
            projectPoints(realCornersArray[i],
                          rvecArray[j],
                          tvecArray[j],
                          cameraMatrix,
                          distortionCoefficient,
                          projectionCorners);

            err = cv::norm(cornersArray[i], projectionCorners, cv::NORM_L2);
            size_t n = realCornersArray[i].size();
            reprojectionErrorsArray[i] = std::sqrt(err * err / n);
            totalErr += err * err;
            totalPoints += n;
            j++;
        }
    }

    avgError2 = std::sqrt(totalErr / totalPoints);
}

void Calibration::undistortCalibrationImages()
{
    undistortedImagesArray.clear();
    undistortedImagesArray.resize(imagesArray.size());
    for (int i = 0; i < imagesArray.size(); ++i)
    {
        if (goodImages[i])
        {
            cv::undistort(imagesArray[i], undistortedImagesArray[i], cameraMatrix, distortionCoefficient);
        }
        else
        {
            undistortedImagesArray[i] = imagesArray[i].clone();
        }
    }
}

void Calibration::drawPattern()
{
    for (int i = 0; i < imagesArray.size(); ++i)
    {
        cv::drawChessboardCorners(imagesArray[i], patternSize, cornersArray[i], goodImages[i]);

        std::vector<cv::Point2f> undistordetCorners;
        if (goodImages[i])
        {
            cv::undistortPoints(cornersArray[i], undistordetCorners, cameraMatrix, distortionCoefficient);
            cv::drawChessboardCorners(undistortedImagesArray[i], patternSize, undistordetCorners, goodImages[i]);
        }
    }
}

cv::Mat Calibration::undistort(const cv::Mat &img)
{
    cv::Mat undistort;
	if (!cameraMatrix.empty() && !distortionCoefficient.empty())
	{
		cv::undistort(img, undistort, cameraMatrix, distortionCoefficient);
		return undistort;
	}
	else
	{
		return cv::Mat();
	}
}

const std::vector<cv::Mat> &Calibration::getImagesArray() const
{
    return imagesArray;
}
const std::vector<bool> &Calibration::getGoodImages() const
{
    return goodImages;
}
const std::vector<double> &Calibration::getReprojectionErrorsArray() const
{
    return reprojectionErrorsArray;
}

const cv::Mat &Calibration::getImage(int i) const
{
    if (i < imagesArray.size())
    {
        return imagesArray[i];
    }
    else
    {
        return empty;
    }
}

const cv::Mat &Calibration::getUndistortedImage(int i) const
{
    if (i < undistortedImagesArray.size())
    {
        return undistortedImagesArray[i];
    }
    else
    {
        return empty;
    }
}

double Calibration::getAvgError2() const
{
    return avgError2;
}



