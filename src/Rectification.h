//
// Created by Piotr Meller on 14.11.18.
//

#ifndef PRACA_INZYNIERSKA_RECTIFICATION_H
#define PRACA_INZYNIERSKA_RECTIFICATION_H

#include <opencv2/core.hpp>
#include <geometry/pose3.hpp>
#include <numeric/numeric.h>
#include <cameras/Camera_Intrinsics.hpp>
#include <sfm/pipelines/sfm_robust_model_estimation.hpp>
#include <robust_estimation/robust_estimator_ACRansacKernelAdaptator.hpp>
#include <multiview/solver_essential_kernel.hpp>
#include <cameras/Camera_Pinhole.hpp>
#include <robust_estimation/robust_estimator_ACRansac.hpp>
#include <multiview/solver_essential_eight_point.hpp>
#include <openMVG/multiview/motion_from_essential.hpp>

class Rectification
{
public:
	enum RejectionMethod
	{
		RANSAC,
		LMEDS,
		RANSAC_POSE,
		LMEDS_POSE,
	};

	enum RectificationMethod
	{
		LOOP_ZHANG,
		HARTLEY,
		DSR,
		POSE_ESTIMATION
	};

	//cv::Mat imageLeft, imageRight;
		cv::Mat leftRotation, rightRotation;

private:
public:

	cv::Mat rectImageLeft, rectImageMiddle, rectImageRight;
	cv::Mat rectImageLeftSizeCorrection, rectImageMiddleSizeCorrection, rectImageRightSizeCorrection;

	std::vector<cv::Point2f> leftPoints, middlePoints, rightPoints;
	std::vector<cv::Point2f> leftFilteredPoints, middleFilteredPoints, rightFilteredPoints;

	cv::Mat relativeTranslation, relativeRotation, Q;



	cv::Mat homLeft, homRight, homLeftCorrection, homRightCorrection;

	//cv::Mat leftRemap[2], middleRemap[2], rightRemap[2];

	cv::Size originalSize, newSize;

	double min_error, max_error, avg_error;

	RectificationMethod rectificationMethod;


	double rejection_threshold, rejection_confidence;
	int rejection_iterations;

public:
	Rectification();
	~Rectification();

	void setPoints(const std::vector<cv::Point2f>& left_points,
	               const std::vector<cv::Point2f>& right_points);
	void setPoints(const std::vector<cv::Point2f>& left_points, const std::vector<cv::Point2f>& middle_points,
	               const std::vector<cv::Point2f>& right_points);

	void rejectOutliers(int method, double threshold, double confidence, int iterations);

	void rectifyImages(int method, const cv::Mat& left_image, const cv::Mat& left_camMatrix,
	                   const cv::Mat& left_distCoeff, const cv::Mat&
	                   right_image, const cv::Mat& rigth_camMatrix, const cv::Mat& right_distCoeff);

	void rectifyImages(int method, const cv::Mat& left_image, const cv::Mat& left_camMatrix,
	                   const cv::Mat& left_distCoeff, const cv::Mat& middle_image, const cv::Mat& middle_camMatrix,
	                   const cv::Mat& middle_distCoeff, const cv::Mat&
	                   right_image, const cv::Mat& rigth_camMatrix, const cv::Mat& right_distCoeff);

	void warp_image(const cv::Mat& left_image, const cv::Mat& left_camMatrix,
	                const cv::Mat& left_distCoeff, const cv::Mat&
	                right_image, const cv::Mat& right_camMatrix, const cv::Mat& right_distCoeff);


	void calcError(std::vector<cv::Point2f> left_points, std::vector<cv::Point2f> right_points);


	const cv::Mat& getRectImageLeft() const;
	const cv::Mat& getRectImageMiddle() const;
	const cv::Mat& getRectImageRight() const;
	double getMin_error() const;
	double getMax_error() const;
	double getAvg_error() const;


private:

	/**
Opencv implementation of hartley rectification
*/
	void Hartley();

	/**
	Opencv implementation of hartley rectification + anti-shearing Loop-Zhang method
	*/
	void LoopZhang();

	void DirectSelfRectification();

	void PoseEstimationRectification(const cv::Mat& leftCameraMatrix, const cv::Mat& rightCameraMatrix);

};

#endif //PRACA_INZYNIERSKA_RECTIFICATION_H
