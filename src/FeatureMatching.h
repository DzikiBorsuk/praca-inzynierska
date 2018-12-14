//
// Created by Piotr Meller on 14.11.18.
//

#ifndef PRACA_INZYNIERSKA_FEAUREMATCHING_H
#define PRACA_INZYNIERSKA_FEAUREMATCHING_H

#include <opencv2/core.hpp>
#include <opencv2/features2d.hpp>
#include <vector>
#include <string>

class FeatureMatching
{

    cv::Mat left,right;
public:
    const cv::Mat &getFeatures() const;
private:
    cv::Mat features;

    cv::Ptr<cv::Feature2D> detector,descriptor;
    cv::Ptr<cv::DescriptorMatcher> matcher;

    cv::Mat descriptor_left, descriptor_right;
    std::vector<cv::KeyPoint> keypoints_left, keypoints_right;
    std::vector<cv::DMatch> matches;
    std::vector<cv::DMatch> good_matches;
    std::vector<cv::Point2f> points_left, points_right;


public:
    FeatureMatching();
    ~FeatureMatching();

    enum Detectors{
        SIFT,
        SURF,
        ORB,
        AKAZE,
        KAZE,
        BRISK,

    };


    void loadImages(const cv::Mat &_left, const cv::Mat &_right);
    void loadLeftImage(const cv::Mat &_left);
    void loadRightImage(const cv::Mat &_right);

    void setDetector(int type,std::vector<double> params);
    void setDescriptor(int type,std::vector<double> params);
    void setMatcher(int type, std::vector<double> params);

    unsigned long getNumOfLeftKeyPoints();
    unsigned long getNumOfRightKeyPoints();
    unsigned long getNumOfMatches();
    const std::vector<cv::Point2f> &getPoints_left() const;
    const std::vector<cv::Point2f> &getPoints_right() const;

    void detectKeypoints();
    void extractDescriptor();
    void matchKeypoints();


};

#endif //PRACA_INZYNIERSKA_FEAUREMATCHING_H
