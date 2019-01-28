//
// Created by Piotr Meller on 20.10.18.
//

#ifndef PRACA_INZYNIERSKA_STEREO_H
#define PRACA_INZYNIERSKA_STEREO_H


#include <vector>
#include <string>
#include "Rectification.h"
#include "FeatureMatching.h"
#include "Calibration.h"
#include "Disparity.h"
#include "ImageStructure.h"

class Stereo
{
public:

    Calibration calib;
    FeatureMatching featureMatching;
    Rectification rect;
    Disparity disp;

private:
public:
	ImageStructure orgLeft, orgMiddle, orgRight;
	ImageStructure left, middle, right;
public:
    const ImageStructure &getLeft() const;
    const ImageStructure &getOrgLeft() const;
    const ImageStructure &getMiddle() const;
    const ImageStructure &getOrgMiddle() const;
    const ImageStructure &getRight() const;
    const ImageStructure &getOrgRight() const;
private:
    //cv::Mat left_disp,right_disp,filtered_disp;
    //bool run_calibration;

    std::vector<cv::KeyPoint> keypoints_left, keypoints_right;
    std::vector<cv::Point2f> lk, rk;


public:
    Stereo() = default;
    //Stereo(const std::string[] img, const std::string &_middle, const std::string &_right, const std::string &_cameraParamsFile="");
    Stereo(const std::string &_left, const std::string &_right, const std::string &_cameraParamsFile = "");

    void loadLeftImage(const std::string &image_path, const std::string &camera_params_path = "");
    void loadRightImage(const std::string &image_path, const std::string &camera_params_path = "");

    void rectify2Image();
    void rectify3Image();

	void equalizeImages();

    void saveRectifiedImages(const std::string &directory);

    void computeDisp();

    void show();

//private:
    void match_feautures();
};


#endif //PRACA_INZYNIERSKA_STEREO_H
