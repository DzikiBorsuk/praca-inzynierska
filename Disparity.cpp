//
// Created by Piotr Meller on 14.11.18.
//

#include "Disparity.h"
#include <opencv2/imgproc.hpp>
#include "opencv2/ximgproc.hpp"
#include "opencv2/ximgproc/disparity_filter.hpp"


Disparity::Disparity()
{
}


Disparity::~Disparity()
{
}

void Disparity::SGBM(const cv::Mat &left, const cv::Mat &right)
{
	
	int min_disp = -160;
	int max_disp = 320;
	int num_disp = max_disp - min_disp;
	int wsize = 5;


	cv::Ptr<cv::StereoSGBM> left_matcher = cv::StereoSGBM::create(min_disp, num_disp, wsize);
	left_matcher->setP1(24 *3* wsize*wsize);
	left_matcher->setP2(96 *3* wsize*wsize);
	left_matcher->setPreFilterCap(63);
	left_matcher->setMode(cv::StereoSGBM::MODE_SGBM_3WAY);
	//left_matcher->setUniquenessRatio(15);
	//left_matcher->setSpeckleWindowSize(1000);
	//left_matcher->setSpeckleRange(16);
	//left_matcher->setDisp12MaxDiff(30);


	cv::Ptr<cv::ximgproc::DisparityWLSFilter> wls_filter;
	wls_filter = cv::ximgproc::createDisparityWLSFilter(left_matcher);
	cv::Ptr<cv::StereoMatcher> right_matcher = cv::ximgproc::createRightMatcher(left_matcher);

	left_matcher->compute(left, right, left_disp);
	right_matcher->compute(right, left, right_disp);

	float lambda = 8000.f;
	float sigma = 1.5f;

	wls_filter->setLambda(lambda);
	wls_filter->setSigmaColor(sigma);

	wls_filter->filter(left_disp, left, filtered_disp, right_disp,cv::Rect(),right);

	cv::Mat vis_left, vis_right, vis_filter;
	cv::ximgproc::getDisparityVis(left_disp, vis_left, 1);
	cv::ximgproc::getDisparityVis(right_disp, vis_right, -1);
	cv::ximgproc::getDisparityVis(filtered_disp, vis_filter, 1);
}
