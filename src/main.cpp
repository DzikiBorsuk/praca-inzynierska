//
// Created by Piotr Meller on 15.10.18.
//

#define OPENCV_TRAITS_ENABLE_DEPRECATE
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include "Stereo.h"
#include <opencv2/highgui.hpp>


using namespace cv;

int main()
{
	//Stereo stereo("n_left.jpg","n_right.jpg");
	//Stereo stereo("2.JPG", "3.JPG", "out_camera_data.xml");

	// int last_i;
	// std::cout<<"number of photo"<<std::endl;
	// std::cin >> last_i;
	//
	// std::string path = "";
	// for (int i = 1; i <= last_i; i++)
	// {
	//     std::string temp = "";
	//     if (i < 10)
	//     {
	//         temp = temp + "0";
	//     }
	//     temp = temp + std::to_string(i);
	//     std::string load = path + temp + ".JPG";
	//     Mat src = imread(load);
	//
	//     int width = src.size().width;
	//     int height = src.size().height;
	//
	//     Rect r_left(0, 0, width / 2, height);
	//     Rect r_right(width / 2, 0, width / 2, height);
	//
	//
	//     Mat left_ = src(r_left);
	//     Mat right_ = src(r_right);
	//     Mat left, right;
	//     cv::resize(left_, left, { width, height });
	//     cv::resize(right_, right, { width, height });
	//     imwrite(path + "left/" + temp + ".png", left);
	//     imwrite(path + "right/" + temp + ".png", right);
	// }


	//Stereo stereo("n_left.jpg","n_right.jpg");
	//    Stereo stereo("2.JPG", "3.JPG", "out_camera_data.xml");
	//
	//    stereo.match_feautures();
	//    stereo.rectifyImage();
	//    stereo.computeDisp();

	


	Stereo stereo;

	Mat leftCam, leftDist, rightCam, rightDist;

	stereo.loadLeftImage("l.JPG", "out_camera_data.xml");
	stereo.loadRightImage("s.JPG", "out_camera_data.xml");
	
	//stereo.loadLeftImage("zdj/8/l.png", "zdj/p9.yml");
	//stereo.loadRightImage("zdj/8/r.png", "zdj/s9.yml");



	stereo.featureMatching.setDetector(FeatureMatching::Detectors::SURF, {100, 4, 3, 0});
	stereo.featureMatching.setDescriptor(FeatureMatching::Detectors::SURF, {100, 4, 3, 0});
	stereo.featureMatching.setMatcher(0, std::vector<double>());
	stereo.featureMatching.detect2Keypoints();
	stereo.featureMatching.extract2Descriptor();

	stereo.featureMatching.match2Keypoints();

	stereo.rect.setPoints(stereo.featureMatching.getPoints_left(), stereo.featureMatching.getPoints_right());
	//stereo.rect.setCameraMatrix(stereo.calib.getCameraMatrix());

	stereo.rect.rejectOutliers(Rectification::RejectionMethod::RANSAC, 7, 0.995, 2000);
	//stereo.rect.rectifyImages(Rectification::RectificationMethod::DSR);
	//stereo.rect.estimatePose();
	//stereo.rect.rectifyImages(Rectification::RectificationMethod::POSE_ESTIMATION);

	// stereo.rect.rectifyImages(Rectification::RectificationMethod::POSE_ESTIMATION, stereo.getOrgLeft(),
	//                           leftCam, leftDist,
	//                           stereo.getOrgRight(),
	//                           rightCam, rightDist);

	stereo.rect.rectifyImages(Rectification::RectificationMethod::POSE_ESTIMATION, stereo.getOrgLeft().image,
	                          stereo.getOrgLeft().cameraMatrix, stereo.getOrgLeft().distortionCoefficient,
	                          stereo.getOrgRight().image,
	                          stereo.getOrgRight().cameraMatrix, stereo.getOrgRight().distortionCoefficient);


	stereo.disp.initImages(stereo.rect.getRectImageLeft(), stereo.rect.getRectImageRight());

	//stereo.disp.initSGBM(-5,320,5,150,3500);
	stereo.disp.initSGBM(0,80,3,150,3500);

	stereo.disp.SGBM();

	stereo.disp.filterDisparity();

	cv::Mat a = stereo.disp.getDisparity(2);
	cv::Mat b = stereo.disp.getDisparityFiltered(2);


	// cv::Mat R1,R2,P1,P2,Q;
	//
	// stereo.calib.getCameraMatrix();
	//
	// cv::stereoRectify(stereo.calib.getCameraMatrix(), stereo.calib.getDistortionCoefficient(),
	//                   stereo.calib.getCameraMatrix(), stereo.calib.getDistortionCoefficient(), stereo.orgLeft.size(),
	//                   stereo.rect.relativeRotation, stereo.rect.relativeTranslation,R1,R2,P1,P2,Q);
	//
	// Mat rmap[2][2];
	//
	// cv::Mat F = cv::findFundamentalMat(stereo.rect.leftFilteredPoints, stereo.rect.rightFilteredPoints,cv::FM_8POINT);
	//
	// cv::Mat H1, H2;
	//
	// cv::stereoRectifyUncalibrated(stereo.rect.leftFilteredPoints, stereo.rect.rightFilteredPoints, F, stereo.orgLeft.size(), H1, H2);
	//
	// //stereo.rect.warp_image();
	//
	// H1 = stereo.rect.homLeftCorrection;
	// H2 = stereo.rect.homRightCorrection;
	//
	// R1 = stereo.calib.getCameraMatrix().inv()*H1*stereo.calib.getCameraMatrix();
	// R2 = stereo.calib.getCameraMatrix().inv()*H2*stereo.calib.getCameraMatrix();
	// P1 = stereo.calib.getCameraMatrix();
	// P2 = stereo.calib.getCameraMatrix();
	//
	//
	// cv::initUndistortRectifyMap(stereo.calib.getCameraMatrix(), stereo.calib.getDistortionCoefficient(), R1, P1, stereo.rect.newSize, CV_16SC2, rmap[0][0], rmap[0][1]);
	// cv::initUndistortRectifyMap(stereo.calib.getCameraMatrix(), stereo.calib.getDistortionCoefficient(), R2, P2, stereo.rect.newSize, CV_16SC2, rmap[1][0], rmap[1][1]);
	//
	//
	//
	// cv::Mat l, r;
	//
	// remap(stereo.orgLeft, l, rmap[0][0], rmap[0][1], INTER_LINEAR);
	// remap(stereo.orgRight, r, rmap[1][0], rmap[1][1], INTER_LINEAR);

	//stereo.rect.warp_image();


	cv::imwrite("2r.png", stereo.rect.getRectImageLeft());
	cv::imwrite("1r.png", stereo.rect.getRectImageRight());


	//stereo.match_feautures();
	//stereo.rectifyImage();
	//stereo.computeDisp();
}
