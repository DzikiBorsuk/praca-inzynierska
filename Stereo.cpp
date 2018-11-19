//
// Created by Piotr Meller on 20.10.18.
//

#include <iostream>
#include <stdio.h>

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



Stereo::Stereo(const std::string & _left, const std::string & _right, const std::string & _cameraParamsFile) :calib(_cameraParamsFile)
{
	left = cv::imread(_left);
	right = cv::imread(_right);
	left = calib.undistort(left);
	right = calib.undistort(right);
}

void Stereo::rectifyImage()
{
	rect.DSR(left, lk, right, rk);
}

void Stereo::computeDisp()
{


	//disp.SGBM(rect.getLeft(), rect.getRight());

	cv::Mat l, r;
	cv::resize(rect.getLeft(), l, {1920,1080});
	cv::resize(rect.getRight(), r, { 1920,1080 });

	disp.SGBM(l, r);

}

void Stereo::show()
{

}

void Stereo::match_feautures()
{
	if (!left.data || !right.data)
	{
		std::cout << " --(!) Error reading images " << std::endl;
		return;
	}
	//-- Step 1: Detect the keypoints using SURF Detector, compute the descriptors
	int minHessian = 400;
	cv::Ptr<cv::xfeatures2d::SURF> detector = cv::xfeatures2d::SURF::create();
	detector->setHessianThreshold(minHessian);
	cv::Mat descriptors_1, descriptors_2;
	detector->detectAndCompute(left, cv::Mat(), keypoints_left, descriptors_1);
	detector->detectAndCompute(right, cv::Mat(), keypoints_right, descriptors_2);
	//-- Step 2: Matching descriptor vectors using FLANN matcher
	cv::FlannBasedMatcher matcher;
	std::vector<cv::DMatch> matches;
	matcher.match(descriptors_1, descriptors_2, matches);
	double max_dist = 0;
	double min_dist = 200;
	//-- Quick calculation of max and min distances between keypoints
	for (int i = 0; i < descriptors_1.rows; i++)
	{
		double dist = matches[i].distance;
		if (dist < min_dist) { min_dist = dist; }
		if (dist > max_dist) { max_dist = dist; }
	}
	printf("-- Max dist : %f \n", max_dist);
	printf("-- Min dist : %f \n", min_dist);
	//-- Draw only "good" matches (i.e. whose distance is less than 2*min_dist,
	//-- or a small arbitrary value ( 0.02 ) in the event that min_dist is very
	//-- small)
	//-- PS.- radiusMatch can also be used here.
	std::vector<cv::DMatch> good_matches;
	std::vector<int> good_l;
	std::vector<int> good_r;
	for (int i = 0; i < descriptors_1.rows; i++)
	{
		if (matches[i].distance <= std::max(2 * min_dist, 0.05))
		{
			good_matches.push_back(matches[i]);
			//good.push_back(i);//Todo:  poprawic

			good_l.push_back(matches[i].queryIdx);
			good_r.push_back(matches[i].trainIdx);
		}
	}
	//-- Draw only "good" matches
	cv::Mat img_matches;
	drawMatches(left, keypoints_left, right, keypoints_right,
		good_matches, img_matches, cv::Scalar::all(-1), cv::Scalar::all(-1),
		std::vector<char>(), cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
	//-- Show detected matches

	cv::KeyPoint::convert(keypoints_left, lk, good_l);//Todo:  poprawic
	cv::KeyPoint::convert(keypoints_right, rk, good_r);


	//cv::namedWindow("Good Matches", cv::WINDOW_NORMAL);
	//cv::imshow("Good Matches", img_matches);
	//cv::resizeWindow("Good Matches", 1200, 900);
	for (int i = 0; i < (int)good_matches.size(); i++)
	{
		printf("-- Good Match [%d] Keypoint 1: %d  -- Keypoint 2: %d  \n", i, good_matches[i].queryIdx,
			good_matches[i].trainIdx);
	}
	//cv::waitKey(0);
	std::cout << "a" << std::endl;
}


