//
// Created by Piotr Meller on 14.11.18.
//

#include "FeatureMatching.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/xfeatures2d.hpp>


FeatureMatching::FeatureMatching()
{
}

FeatureMatching::~FeatureMatching()
{
}

void FeatureMatching::loadImages(const cv::Mat& _left, const cv::Mat& _right)
{
	left = _left.clone();
	right = _right.clone();
	cv::hconcat(left, right, features);
}

void FeatureMatching::loadLeftImage(const cv::Mat& _left)
{
	left = _left.clone();
	if (left.size() != right.size())
	{
		if (right.empty())
		{
			right = cv::Mat::zeros(left.size(), CV_8UC3);
		}
		else
		{
			cv::Mat temp = right.clone();
			cv::resize(temp, right, left.size());
		}
	}

	cv::hconcat(left, right, features);
}

void FeatureMatching::loadMiddleImage(const cv::Mat& _middle)
{
}

void FeatureMatching::loadRightImage(const cv::Mat& _right)
{
	right = _right.clone();
	if (right.size() != left.size())
	{
		if (left.empty())
		{
			left = cv::Mat::zeros(right.size(), CV_8UC3);
		}
		else
		{
			cv::Mat temp = left.clone();
			cv::resize(temp, left, right.size());
		}
	}

	cv::hconcat(left, right, features);
}

void FeatureMatching::setDetector(int type, std::vector<double> params)
{
	switch (type)
	{
	case Detectors::SIFT:
		detector = cv::xfeatures2d::SIFT::create(params[0], params[1], params[2], params[3], params[4]);
		break;
	case Detectors::SURF:
		detector = cv::xfeatures2d::SURF::create(params[0], params[1], params[2], false, params[3]);
		break;
	case Detectors::ORB:
		detector = cv::ORB::create(params[0], params[1], params[2], params[3], params[4], params[5], params[6],
		                           params[7], params[8]);
		break;
	case Detectors::AKAZE:
		detector = cv::AKAZE::create(params[0], params[1], params[2], params[3], params[4], params[5], params[6]);
		break;
	case Detectors::KAZE:
		detector = cv::KAZE::create(params[0], params[1], params[2], params[3], params[4], params[5]);
		break;
	case Detectors::BRISK:
		detector = cv::BRISK::create(params[0], params[1], params[2]);
		break;
	}
}

void FeatureMatching::setDescriptor(int type, std::vector<double> params)
{
	switch (type)
	{
	case Detectors::SIFT:
		descriptor = cv::xfeatures2d::SIFT::create(params[0], params[1], params[2], params[3], params[4]);
		break;
	case Detectors::SURF:
		descriptor = cv::xfeatures2d::SURF::create(params[0], params[1], params[2], false, params[3]);
		break;
	case Detectors::ORB:
		descriptor = cv::ORB::create(params[0], params[1], params[2], params[3], params[4], params[5], params[6],
		                             params[7], params[8]);
		break;
	case Detectors::AKAZE:
		descriptor = cv::AKAZE::create(params[0], params[1], params[2], params[3], params[4], params[5], params[6]);
		break;
	case Detectors::KAZE:
		descriptor = cv::KAZE::create(params[0], params[1], params[2], params[3], params[4], params[5]);
		break;
	case Detectors::BRISK:
		descriptor = cv::BRISK::create(params[0], params[1], params[2]);
		break;
	}
}


void FeatureMatching::setMatcher(int type, std::vector<double> params)
{
	matcher = cv::DescriptorMatcher::create(1);
	if (!params.empty())
	{
		setMinDistance(params[0]);
	}
	else { setMinDistance(0.1); }
}

void FeatureMatching::setMinDistance(double min_distance)
{
	this->min_distance = min_distance;
}

unsigned long FeatureMatching::getNumOfLeftKeyPoints()
{
	return keypoints_left.size();
}

unsigned long FeatureMatching::getNumOfRightKeyPoints()
{
	return keypoints_right.size();
}

unsigned long FeatureMatching::getNumOfMatches()
{
	return good_matches.size();
}

const std::vector<cv::Point2f>& FeatureMatching::getPoints_left() const
{
	return points_left;
}

const std::vector<cv::Point2f>& FeatureMatching::getPoints_right() const
{
	return points_right;
}


void FeatureMatching::detect2Keypoints()
{
	detector->detect(left, keypoints_left);
	detector->detect(right, keypoints_right);
}

void FeatureMatching::extract2Descriptor()
{
	descriptor->compute(left, keypoints_left, descriptor_left);
	descriptor->compute(right, keypoints_right, descriptor_right);
}

void FeatureMatching::match2Keypoints()
{
	matches.clear();
	good_matches.clear();

	if (descriptor_left.type() != CV_32F)
	{
		descriptor_left.convertTo(descriptor_left, CV_32F);
	}

	if (descriptor_right.type() != CV_32F)
	{
		descriptor_right.convertTo(descriptor_right, CV_32F);
	}

	matcher->match(descriptor_left, descriptor_right, matches);
	double max_dist = 0;
	double min_dist = 200;
	//-- Quick calculation of max and min distances between keypoints
	for (int i = 0; i < descriptor_left.rows; i++)
	{
		double dist = matches[i].distance;
		if (dist < min_dist)
		{
			min_dist = dist;
		}
		if (dist > max_dist)
		{
			max_dist = dist;
		}
	}
	printf("-- Max dist : %f \n", max_dist);
	printf("-- Min dist : %f \n", min_dist);
	//-- Draw only "good" matches (i.e. whose distance is less than 2*min_dist,
	//-- or a small arbitrary value ( 0.02 ) in the event that min_dist is very
	//-- small)
	//-- PS.- radiusMatch can also be used here.
	std::vector<int> good_l;
	std::vector<int> good_r;
	for (int i = 0; i < descriptor_left.rows; i++)
	{
		if (matches[i].distance <= std::max(2 * min_dist, this->min_distance))
		{
			good_matches.push_back(matches[i]);
			//good.push_back(i);//Todo:  poprawic

			good_l.push_back(matches[i].queryIdx);
			good_r.push_back(matches[i].trainIdx);
		}
	}
	//-- Draw only "good" matches
	drawMatches(left, keypoints_left, right, keypoints_right,
	            good_matches, features, cv::Scalar::all(-1), cv::Scalar::all(-1),
	            std::vector<char>(), cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
	//-- Show detected matches

	cv::KeyPoint::convert(keypoints_left, points_left, good_l); //Todo:  poprawic
	cv::KeyPoint::convert(keypoints_right, points_right, good_r);
}


const cv::Mat& FeatureMatching::getFeatures() const
{
	return features;
}
