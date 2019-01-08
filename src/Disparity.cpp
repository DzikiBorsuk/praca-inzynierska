//
// Created by Piotr Meller on 14.11.18.
//

#include "Disparity.h"
#include <opencv2/imgproc.hpp>
#include "opencv2/ximgproc.hpp"
#include "opencv2/ximgproc/disparity_filter.hpp"
#include <opencv2/highgui.hpp>

#include <iostream>


Disparity::Disparity()
{
	//initSGBM();
	left_matcher = cv::StereoSGBM::create();
	right_matcher = cv::ximgproc::createRightMatcher(left_matcher);
	contrast = 1;
}

void Disparity::initSGBM(int _minDisparity,
                         int _numDisparities,
                         int _blockSize,
                         int _P1,
                         int _P2,
                         int _disp12MaxDiff,
                         int _preFilterCap,
                         int _uniquenessRatio,
                         int _speckleWindowSize,
                         int _speckleRange,
                         int _mode)
{
	minDisparity = _minDisparity;
	numDisparity = _numDisparities;
	blockSize = _blockSize;
	P1 = _P1;
	P2 = _P2;
	disp12MaxDiff = _disp12MaxDiff;
	preFilterCap = _preFilterCap;
	uniquenessRatio = _uniquenessRatio;
	speckleWindowSize = _speckleWindowSize;
	speckleRange = _speckleRange;

	setMatcher();
}

void Disparity::initImages(const cv::Mat& left, const cv::Mat& right)
{
	//this->left_image = left.clone();
	//this->right_image = right.clone();
	double scale = 1.0 * 800 / left.size().width;
	cv::resize(left, left_image, cv::Size(), scale, scale);
	cv::resize(right, right_image, cv::Size(), scale, scale);
}


Disparity::~Disparity()
= default;

void Disparity::SGBM()
{
	setMatcher();




	// left_matcher->setMinDisparity(-16);
	// left_matcher->setNumDisparities(320 + 32);
	// left_matcher->setP1(24 * 3 * 3);
	// left_matcher->setP2(96 * 3 * 3);
	// left_matcher->setPreFilterCap(63);

	//right_matcher = cv::ximgproc::createRightMatcher(left_matcher);

	//right_matcher = cv::StereoSGBM::create(-320+5,320,5,24*5*5,96*5*5);

	std::cout << "disp" << std::endl;
	left_matcher->compute(left_image, right_image, left_disp);
	right_matcher->compute(right_image, left_image, right_disp);

	std::cout << "disp_finish" << std::endl;
	wls_filter = cv::ximgproc::createDisparityWLSFilter(left_matcher);

	//cv::ximgproc::getDisparityVis(left_disp, vis_left, contrast);
	//cv::ximgproc::getDisparityVis(right_disp, vis_right, -contrast);
}

void Disparity::filterDisparity()
{
	float lambda = 8000.f;
	float sigma = 1.5f;

	wls_filter->setLambda(lambda);
	wls_filter->setSigmaColor(sigma);

	wls_filter->filter(left_disp, left_image, filtered_disp, right_disp, cv::Rect(), right_image);

	//cv::ximgproc::getDisparityVis(filtered_disp, vis_filter, contrast);

	//cv::applyColorMap(vis_filter, vis_filter, cv::COLORMAP_JET);
}

const cv::Mat& Disparity::getDisparity(int color)
{
	if (!left_disp.empty())
	{
		cv::ximgproc::getDisparityVis(left_disp, vis_left, contrast);
		if (color > 0 && color <= 12)
		{
			cv::applyColorMap(vis_left, vis_left, color);
		}
	}
	return vis_left;
}

const cv::Mat& Disparity::getDisparityFiltered(int color)
{
	if (!filtered_disp.empty())
	{
		cv::ximgproc::getDisparityVis(filtered_disp, vis_filter, contrast);
		if (color > 0 && color <= 12)
		{
			cv::applyColorMap(vis_filter, vis_filter, color);
		}
	}
	return vis_filter;
}
