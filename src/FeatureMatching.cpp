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

void FeatureMatching::loadImages(const cv::Mat &_left, const cv::Mat &_right)
{
    left = _left.clone();
    right = _right.clone();
    cv::hconcat(left, right, features);
}
void FeatureMatching::loadLeftImage(const cv::Mat &_left)
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
void FeatureMatching::loadRightImage(const cv::Mat &_right)
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
    detector = cv::xfeatures2d::SIFT::create();
}
void FeatureMatching::setDescriptor(int type, std::vector<double> params)
{
    descriptor = cv::xfeatures2d::SIFT::create();
}
void FeatureMatching::setMatcher(const std::string &type, std::vector<double> params)
{
    matcher = cv::DescriptorMatcher::create("FlannBased");
}

void FeatureMatching::detectKeypoints()
{
    detector->detect(left, keypoints_left);
    detector->detect(right, keypoints_right);
}

void FeatureMatching::extractDescriptor()
{
    detector->compute(left, keypoints_left, descriptor_left);
    detector->compute(right, keypoints_right, descriptor_right);
}

void FeatureMatching::matchKeypoints()
{
    matcher->match(descriptor_left, descriptor_right, matches);
    double max_dist = 0;
    double min_dist = 200;
    //-- Quick calculation of max and min distances between keypoints
    for (int i = 0; i < descriptor_left.rows; i++)
    {
        double dist = matches[i].distance;
        if (dist < min_dist)
        { min_dist = dist; }
        if (dist > max_dist)
        { max_dist = dist; }
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
    for (int i = 0; i < descriptor_left.rows; i++)
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
    drawMatches(left, keypoints_left, right, keypoints_right,
                good_matches, features, cv::Scalar::all(-1), cv::Scalar::all(-1),
                std::vector<char>(), cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
    //-- Show detected matches

    cv::KeyPoint::convert(keypoints_left, points_left, good_l);//Todo:  poprawic
    cv::KeyPoint::convert(keypoints_right, points_right, good_r);
}
const cv::Mat &FeatureMatching::getFeatures() const
{
    return features;
}


