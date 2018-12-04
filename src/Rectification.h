//
// Created by Piotr Meller on 14.11.18.
//

#ifndef PRACA_INZYNIERSKA_RECTIFICATION_H
#define PRACA_INZYNIERSKA_RECTIFICATION_H

#include <opencv2/core.hpp>

class Rectification
{
	cv::Mat rect_left;
	cv::Mat rect_right;

	cv::Mat homLeft, homRight, homLeftCorrection, homRightCorrection;

	cv::Size originalSize;

	double min_error, max_error, avg_error;

public:
	Rectification();
	~Rectification();


	/**
	Opencv implementation of hartley rectification
	*/
	void Hartley(const cv::Mat & left, std::vector<cv::Point2f> left_points, const cv::Mat & right, std::vector<cv::Point2f> right_points);

	/**
	Opencv implementation of hartley rectification + anti-shearing Loop-Zhang method
	*/
	void LoopZhang(const cv::Mat & left, std::vector<cv::Point2f> left_points, const cv::Mat & right, std::vector<cv::Point2f> right_points);


	void DSR(const cv::Mat & left, std::vector<cv::Point2f> left_points, const cv::Mat & right, std::vector<cv::Point2f> right_points);




	cv::Mat getLeft();
	cv::Mat getRight();

	void calcError(std::vector<cv::Point2f> left_points, std::vector<cv::Point2f> right_points);


private:
	void warp_image(const cv::Mat & left, const cv::Mat & right);


};

#endif //PRACA_INZYNIERSKA_RECTIFICATION_H
