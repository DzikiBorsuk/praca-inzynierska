//
// Created by Piotr Meller on 14.11.18.
//

#ifndef PRACA_INZYNIERSKA_DISPARITY_H
#define PRACA_INZYNIERSKA_DISPARITY_H

#include <opencv2/core.hpp>

class Disparity
{
	cv::Mat left_disp, right_disp, filtered_disp;

public:
	Disparity();
	~Disparity();

	void SGBM(const cv::Mat &left, const cv::Mat &right);
};

#endif //PRACA_INZYNIERSKA_DISPARITY_H

