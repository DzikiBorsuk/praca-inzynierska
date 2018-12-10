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
    left_matcher = cv::StereoSGBM::create();
    right_matcher = cv::ximgproc::createRightMatcher(left_matcher);
}

void Disparity::initSGBM(int _minDisparity,
                         int _maxDisparities,
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
    maxDisparity = _maxDisparities;
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

Disparity::~Disparity()
= default;

void Disparity::SGBM(const cv::Mat &left, const cv::Mat &right)
{

    setMatcher();


    std::cout << "disp" << std::endl;
    left_matcher->compute(left, right, left_disp);
    right_matcher->compute(right, left, right_disp);

    std::cout << "disp_finish" << std::endl;
    cv::Ptr<cv::ximgproc::DisparityWLSFilter> wls_filter;
    wls_filter = cv::ximgproc::createDisparityWLSFilter(left_matcher);

    float lambda = 8000.f;
    float sigma = 1.5f;

    wls_filter->setLambda(lambda);
    wls_filter->setSigmaColor(sigma);

    wls_filter->filter(left_disp, left, filtered_disp, right_disp, cv::Rect(), right);

    cv::ximgproc::getDisparityVis(left_disp, vis_left, 3);
    cv::ximgproc::getDisparityVis(right_disp, vis_right, -1);
    cv::ximgproc::getDisparityVis(filtered_disp, vis_filter, 1);

}
