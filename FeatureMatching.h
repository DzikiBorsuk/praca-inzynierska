//
// Created by Piotr Meller on 14.11.18.
//

#ifndef PRACA_INZYNIERSKA_FEAUREMATCHING_H
#define PRACA_INZYNIERSKA_FEAUREMATCHING_H

#include <opencv2/core.hpp>

class FeatureMatching
{
	std::vector<cv::KeyPoint> keypoints_left, keypoints_right;
	std::vector<cv::Point2f> lk, rk;

public:
	FeatureMatching();
	~FeatureMatching();


};

#endif //PRACA_INZYNIERSKA_FEAUREMATCHING_H
