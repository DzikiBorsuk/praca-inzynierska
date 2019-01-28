//
// Created by Piotr Meller on 20.10.18.
//

#include <iostream>
#include <cstdio>

#include "Stereo.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/features2d.hpp>
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/core/utility.hpp"
#include "opencv2/ximgproc.hpp"
#include "opencv2/xfeatures2d.hpp"
#include <vector>
#include <algorithm>


//Stereo::Stereo(const std::string &_left, const std::string &_middle, const std::string &_right,
//               const std::string &_cameraParamsFile)
//    : calib(_cameraParamsFile)
//{
//    left = cv::imread(_left);
//    middle = cv::imread(_middle);
//    right = cv::imread(_right);
//
//    numberOfImage = imgNum::three;
//
//
//    if (!_cameraParamsFile.empty()) {
//        left = calib.undistort(left);
//        middle = calib.undistort(middle);
//        right = calib.undistort(right);
//    }
//}

Stereo::Stereo(const std::string& _left, const std::string& _right, const std::string& _cameraParamsFile)
{
	// left = cv::imread(_left);
	// right = cv::imread(_right);
	//
	//
	// if (!_cameraParamsFile.empty())
	// {
	//     left = calib.undistort(left);
	//     right = calib.undistort(right);
	// }

	loadLeftImage(_left, _cameraParamsFile);
	loadRightImage(_right, _cameraParamsFile);
}

void Stereo::loadLeftImage(const std::string& image_path, const std::string& camera_params_path)
{
	orgLeft.image = cv::imread(image_path);
	if (!camera_params_path.empty())
	{
		calib.loadParams(camera_params_path);
		if (!calib.getCameraMatrix().empty())
		{
			orgLeft.cameraMatrix = calib.getCameraMatrix().clone();
			orgLeft.distortionCoefficient = calib.getDistortionCoefficient().clone();
			left.distortionCoefficient = calib.getDistortionCoefficient().clone();
			left.cameraMatrix = calib.getCameraMatrixAfterUndistortion().clone();
		}
	}
	left.image = calib.undistort(orgLeft.image);
	if (left.image.empty())
	{
		left = orgLeft;
	}
	//equalizeImages();
	featureMatching.loadLeftImage(left.image);
}

void Stereo::loadRightImage(const std::string& image_path, const std::string& camera_params_path)
{
	orgRight.image = cv::imread(image_path);
	if (!camera_params_path.empty())
	{
		calib.loadParams(camera_params_path);
		if (!calib.getCameraMatrix().empty())
		{
			orgRight.cameraMatrix = calib.getCameraMatrix().clone();
			orgRight.distortionCoefficient = calib.getDistortionCoefficient().clone();
			right.distortionCoefficient = calib.getDistortionCoefficient().clone();
			right.cameraMatrix = calib.getCameraMatrixAfterUndistortion().clone();
		}
	}
	right.image = calib.undistort(orgRight.image);
	if (right.image.empty())
	{
		right = orgRight;
	}
	//equalizeImages();
	featureMatching.loadRightImage(right.image);
}

void Stereo::rectify2Image()
{
}

void Stereo::rectify3Image()
{
	//TODO implementation
}

void Stereo::equalizeImages()
{
	cv::Size leftSize = orgLeft.image.size();
	cv::Size rightSize = orgRight.image.size();

	if (!orgLeft.image.empty() && !orgLeft.image.empty())
	{
		if (leftSize.width != rightSize.width)
		{
			if (leftSize.width > rightSize.width)
			{
				orgLeft.resize(rightSize);
				left.resize(rightSize);
			}
			else
			{
				orgRight.resize(leftSize);
				right.resize(leftSize);
			}
		}
	}

	featureMatching.loadImages(left.image, right.image);
}

void Stereo::saveRectifiedImages(const std::string& directory)
{
	cv::imwrite(directory + "/left.png", rect.getRectImageLeft());
	cv::imwrite(directory + "/right.png", rect.getRectImageRight());

	cv::FileStorage fs(directory + "/rectification_data.yml", cv::FileStorage::WRITE);

	fs << "camera_matrix" << calib.getCameraMatrix();
	fs << "distortion_coefficients" << calib.getDistortionCoefficient();
	fs << "relativeRotation" << rect.relativeRotation;
	fs << "relativeTranslation" << rect.relativeTranslation;
	fs << "leftRotation" << rect.leftRotation;
	fs << "rightRotation" << rect.rightRotation;

	fs.release();
}

void Stereo::computeDisp()
{
	//disp.SGBM(rect.getLeft(), rect.getRight());

	//cv::Mat l, r;
	//cv::resize(rect.getLeft(), l, {1920, 1080});
	//cv::resize(rect.getRight(), r, {1920, 1080});

	//disp.SGBM(left, right);
}

void Stereo::show()
{
}

void Stereo::match_feautures()
{
//TODO: implementation
}

const ImageStructure& Stereo::getLeft() const
{
	return left;
}

const ImageStructure& Stereo::getOrgLeft() const
{
	return orgLeft;
}

const ImageStructure& Stereo::getMiddle() const
{
	return middle;
}

const ImageStructure& Stereo::getOrgMiddle() const
{
	return orgMiddle;
}

const ImageStructure& Stereo::getRight() const
{
	return right;
}

const ImageStructure& Stereo::getOrgRight() const
{
	return orgRight;
}
