//
// Created by Piotr Meller on 14.11.18.
//

#include "Disparity.h"
#include <opencv2/imgproc.hpp>
#include "opencv2/ximgproc.hpp"
#include "opencv2/ximgproc/disparity_filter.hpp"


Disparity::Disparity()
{
    left_matcher = cv::StereoSGBM::create();
}

void Disparity::initSGBM(int minDisparity,
                         int numDisparities,
                         int blockSize,
                         int P1,
                         int P2,
                         int disp12MaxDiff,
                         int preFilterCap,
                         int uniquenessRatio,
                         int speckleWindowSize,
                         int speckleRange,
                         int mode)
{
    setMatcher();
}

Disparity::~Disparity()
{
}

void Disparity::SGBM(const cv::Mat &left, const cv::Mat &right)
{


    cv::Ptr<cv::ximgproc::DisparityWLSFilter> wls_filter;
    wls_filter = cv::ximgproc::createDisparityWLSFilter(left_matcher);

    left_matcher->compute(left, right, left_disp);
    right_matcher->compute(right, left, right_disp);

    float lambda = 8000.f;
    float sigma = 1.5f;

    wls_filter->setLambda(lambda);
    wls_filter->setSigmaColor(sigma);

    wls_filter->filter(left_disp, left, filtered_disp, right_disp, cv::Rect(), right);

    cv::Mat vis_left, vis_right, vis_filter;
    cv::ximgproc::getDisparityVis(left_disp, vis_left, 1);
    cv::ximgproc::getDisparityVis(right_disp, vis_right, -1);
    cv::ximgproc::getDisparityVis(filtered_disp, vis_filter, 1);
}
