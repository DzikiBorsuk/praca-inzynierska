//
// Created by Piotr Meller on 14.11.18.
//

#include "Rectification.h"
#include <opencv2/ccalib.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <vector>


Rectification::Rectification()
= default;

Rectification::~Rectification()
= default;


void Rectification::setPoints(const std::vector<cv::Point2f>& left_points, const std::vector<cv::Point2f>& right_points)
{
	leftPoints = left_points;
	rightPoints = right_points;
	leftFilteredPoints = left_points;
	rightFilteredPoints = right_points;

}

void Rectification::setPoints(const std::vector<cv::Point2f>& left_points,
                              const std::vector<cv::Point2f>& middle_points,
                              const std::vector<cv::Point2f>& right_points)
{
	//TODO: implementation
}

void Rectification::rejectOutliers(int method, double threshold, double confidence, int iterations)
{
	rejection_threshold = threshold;
	rejection_confidence = confidence;
	rejection_iterations = iterations;

	std::vector<int> inliers;
	switch (method)
	{
	case RejectionMethod::RANSAC:
		cv::findHomography(leftPoints,
		                   rightPoints,
		                   cv::RANSAC,
		                   threshold,
		                   inliers,
		                   iterations,
		                   confidence);

		break;
	case RejectionMethod::LMEDS:
		cv::findHomography(leftPoints,
		                   rightPoints,
		                   cv::LMEDS,
		                   threshold,
		                   inliers,
		                   iterations,
		                   confidence);

		break;
	default: inliers.push_back(1);
		inliers.resize(leftPoints.size(), inliers[0]);
		break;
	}

	leftFilteredPoints.clear();
	rightFilteredPoints.clear();
	leftFilteredPoints.reserve(leftPoints.size());
	rightFilteredPoints.reserve(rightPoints.size());

	for (int i = 0; i < inliers.size(); ++i)
	{
		if (inliers[i] != 0)
		{
			leftFilteredPoints.push_back(leftPoints[i]);
			rightFilteredPoints.push_back(rightPoints[i]);
		}
	}
}

void Rectification::rectifyImages(int method, const cv::Mat& left_image, const cv::Mat& left_camMatrix,
                                  const cv::Mat& left_distCoeff, const cv::Mat&
                                  right_image, const cv::Mat& rigth_camMatrix, const cv::Mat& right_distCoeff)
{
	originalSize = left_image.size();


	switch (method)
	{
	case RectificationMethod::LOOP_ZHANG:
		this->LoopZhang();
		rectificationMethod = LOOP_ZHANG;
		break;
	case RectificationMethod::HARTLEY:
		this->Hartley();
		rectificationMethod = HARTLEY;
		break;
	case RectificationMethod::DSR:
		this->DirectSelfRectification();
		rectificationMethod = DSR;
		break;
	case RectificationMethod::POSE_ESTIMATION:
		this->PoseEstimationRectification(left_camMatrix, rigth_camMatrix);
		rectificationMethod = POSE_ESTIMATION;
		break;
	}

	warp_image(left_image, left_camMatrix, left_distCoeff, right_image, rigth_camMatrix, right_distCoeff);
}

void Rectification::rectifyImages(int method, const cv::Mat& left_image, const cv::Mat& left_camMatrix,
                                  const cv::Mat& left_distCoeff, const cv::Mat& middle_image,
                                  const cv::Mat& middle_camMatrix,
                                  const cv::Mat& middle_distCoeff, const cv::Mat& right_image,
                                  const cv::Mat& rigth_camMatrix,
                                  const cv::Mat& right_distCoeff)
{
	//TODO: implementation
}

void Rectification::warp_image(const cv::Mat& left_image, const cv::Mat& left_camMatrix,
                               const cv::Mat& left_distCoeff, const cv::Mat&
                               right_image, const cv::Mat& right_camMatrix, const cv::Mat& right_distCoeff)
{
	cv::Mat rmap[2][2];

	cv::Mat R1, R2, P1, P2;


	if (rectificationMethod == RectificationMethod::POSE_ESTIMATION)
	{
		cv::stereoRectify(left_camMatrix, left_distCoeff,
		                  right_camMatrix, right_distCoeff, originalSize,
		                  relativeRotation, relativeTranslation, R1, R2, P1, P2, Q);

		cv::initUndistortRectifyMap(left_camMatrix, left_distCoeff, R1, P1,
		                            originalSize, CV_16SC2, rmap[0][0], rmap[0][1]);
		cv::initUndistortRectifyMap(right_camMatrix, right_distCoeff, R2, P2,
		                            originalSize, CV_16SC2, rmap[1][0], rmap[1][1]);

		remap(left_image, rectImageLeft, rmap[0][0], rmap[0][1], cv::INTER_LINEAR);
		remap(right_image, rectImageRight, rmap[1][0], rmap[1][1], cv::INTER_LINEAR);

		rectImageLeftSizeCorrection = rectImageLeft;
		rectImageRightSizeCorrection = rectImageRight;

		newSize = originalSize;
	}
	else
	{
		R1 = left_camMatrix.inv() * homLeft * left_camMatrix;
		R2 = right_camMatrix.inv() * homRight * right_camMatrix;
		P1 = left_camMatrix;
		P2 = right_camMatrix;


		cv::initUndistortRectifyMap(left_camMatrix, left_distCoeff, R1, P1,
		                            originalSize, CV_16SC2, rmap[0][0], rmap[0][1]);
		cv::initUndistortRectifyMap(right_camMatrix, right_distCoeff, R2, P2,
		                            originalSize, CV_16SC2, rmap[1][0], rmap[1][1]);

		remap(left_image, rectImageLeft, rmap[0][0], rmap[0][1], cv::INTER_LINEAR);
		remap(right_image, rectImageRight, rmap[1][0], rmap[1][1], cv::INTER_LINEAR);

		//Size correction
		const std::vector<cv::Point2f> left_points =
		{
			cv::Point2f(0, 0), cv::Point2f(originalSize.width - 1, 0), cv::Point2f(0, originalSize.height - 1),
			cv::Point2f(originalSize.width - 1, originalSize.height - 1)
		};
		const std::vector<cv::Point2f>& right_points = left_points;

		std::vector<cv::Point2f> left_points_dst, right_points_dst;

		cv::perspectiveTransform(left_points, left_points_dst, homLeft);
		cv::perspectiveTransform(right_points, right_points_dst, homRight);

		float min_x = INT16_MAX;
		float min_y = INT16_MAX;
		float max_x = INT16_MIN;
		float max_y = INT16_MIN;

		for (int i = 0; i < 4; i++)
		{
			if (left_points_dst[i].x < min_x)
			{
				min_x = left_points_dst[i].x;
			}
			if (left_points_dst[i].x > max_x)
			{
				max_x = left_points_dst[i].x;
			}
			if (left_points_dst[i].y < min_y)
			{
				min_y = left_points_dst[i].y;
			}
			if (left_points_dst[i].y > max_y)
			{
				max_y = left_points_dst[i].y;
			}

			if (right_points_dst[i].x < min_x)
			{
				min_x = right_points_dst[i].x;
			}
			if (right_points_dst[i].x > max_x)
			{
				max_x = right_points_dst[i].x;
			}
			if (right_points_dst[i].y < min_y)
			{
				min_y = right_points_dst[i].y;
			}
			if (right_points_dst[i].y > max_y)
			{
				max_y = right_points_dst[i].y;
			}
		}
		double data[] = {1, 0, -min_x, 0, 1, -min_y, 0, 0, 1};

		cv::Mat temp(3, 3, CV_64F, data);

		homLeftCorrection = temp * homLeft;
		homRightCorrection = temp * homRight;

		newSize = cv::Size(max_x - min_x + 1, max_y - min_y + 1);


		R1 = left_camMatrix.inv() * homLeftCorrection * left_camMatrix;
		R2 = right_camMatrix.inv() * homRightCorrection * right_camMatrix;
		P1 = left_camMatrix;
		P2 = right_camMatrix;


		cv::initUndistortRectifyMap(left_camMatrix, left_distCoeff, R1, P1,
		                            newSize, CV_16SC2, rmap[0][0], rmap[0][1]);
		cv::initUndistortRectifyMap(right_camMatrix, right_distCoeff, R2, P2,
		                            newSize, CV_16SC2, rmap[1][0], rmap[1][1]);

		remap(left_image, rectImageLeftSizeCorrection, rmap[0][0], rmap[0][1], cv::INTER_LINEAR);
		remap(right_image, rectImageRightSizeCorrection, rmap[1][0], rmap[1][1], cv::INTER_LINEAR);
	}

	leftRotation = R1;
	rightRotation = R2;
}


void Rectification::Hartley()
{
	cv::Mat fundMatrix = cv::findFundamentalMat(leftFilteredPoints, rightFilteredPoints, cv::FM_8POINT);
	cv::stereoRectifyUncalibrated(leftFilteredPoints,
	                              rightFilteredPoints,
	                              fundMatrix,
	                              originalSize,
	                              homLeft,
	                              homRight);
}

void Rectification::LoopZhang()
{
	//TODO implementacja LoopZhang

	cv::Mat fundMatrix = cv::findFundamentalMat(leftFilteredPoints, rightFilteredPoints, cv::FM_8POINT);
	cv::stereoRectifyUncalibrated(leftFilteredPoints,
	                              rightFilteredPoints,
	                              fundMatrix,
	                              originalSize,
	                              homLeft,
	                              homRight);

	cv::Size imsize = originalSize;

	cv::Point2d a((imsize.width - 1) / 2.f, 0);
	cv::Point2d b(imsize.width - 1, (imsize.height - 1) / 2.f);
	cv::Point2d c((imsize.width - 1) / 2.f, imsize.height - 1);
	cv::Point2d d(0, (imsize.height - 1) / 2.f);
	std::vector<cv::Point2d> p = {a, b, c, d};
	std::vector<cv::Point2d> pp;
	cv::perspectiveTransform(p, pp, homLeft);

	auto u = pp[1] - pp[3];
	auto v = pp[0] - pp[2];

	double s_a = (imsize.height * imsize.height * u.y * u.y + imsize.width * imsize.width * v.y * v.y)
		/ (imsize.height * imsize.width * (u.y * v.x - u.x * v.y));

	double s_b = (imsize.height * imsize.height * u.x * u.y + imsize.width * imsize.width * v.x * v.y)
		/ (imsize.height * imsize.width * (u.x * v.y - u.y * v.x));

	double data1[] = {s_a, s_b, 0, 0, 1, 0, 0, 0, 1};

	cv::Mat Hs(3, 3, CV_64F, data1);

	homLeft = Hs * homLeft;
}

void Rectification::DirectSelfRectification()
{
	/*
	//cv::Mat fundMatrix = cv::findFundamentalMat(lk, rk, CV_FM_RANSAC, 1);
	//    cv::stereoRectifyUncalibrated(lk, rk, fundMatrix, imageLight.size(), homLeft, homRight);
	//
	//    cv::warpPerspective(imageLight, imageLight, homRight, imageLight.size());
	//    cv::warpPerspective(imageLeft, imageLeft, homLeft, imageLeft.size());
	//
	//
	//    cv::namedWindow("r", cv::WINDOW_NORMAL);
	//    cv::namedWindow("l", cv::WINDOW_NORMAL);
	//
	//    cv::imshow("r", imageLight);
	//    cv::imshow("l", imageLeft);
	//    cv::resizeWindow("r", 1200, 900);
	//    cv::resizeWindow("l", 1200, 900);
	//
	//    cv::imwrite("rl.png",imageLeft);
	//    cv::imwrite("rr.png",imageLight);
	//
	//    cv::waitKey(0);

//    int ransac_iters = 1000;
//    int point_pick = 200;
//
//    cv::RNG random(static_cast<uint64>(cv::getCPUTickCount()));
//
//    std::vector<int> rng_index;
//    rng_index.reserve(point_pick);
//
//    double max_align_rate = 0.f;
//    double align_rate_earlystop = 0.995;
//    cv::Mat Hy = cv::Mat::eye(3, 3, CV_64F);
//    int total_len = leftPoints.size();
//
//    for (int i = 0; i < ransac_iters; i++)
//    {
//        rng_index.clear();
//
//        int j = 0;
//        while (j < point_pick)
//        {
//            int rand_i = random.next() % total_len;
//            if (std::find(rng_index.begin(), rng_index.end(), rand_i) != rng_index.end())
//            {
//                // v contains x
//            }
//            else
//            {
//                rng_index.push_back(rand_i);
//                j++;
//            }
//        }
//
//        cv::Mat A = cv::Mat::zeros(point_pick, 5, CV_64F);
//
//        for (int i = 0; i < point_pick; i++)
//        {
//            //A(i,:) = [pts2(1, i), pts2(2, i), 1, -1*pts2(1, i)*pts1(2, i), -1*pts2(2, i)*pts1(2, i)];
//
//            A.at<double>(i, 0) = rightPoints[rng_index[i]].x;
//            A.at<double>(i, 1) = rightPoints[rng_index[i]].y;
//            A.at<double>(i, 2) = 1;
//            A.at<double>(i, 3) = -1 * rightPoints[rng_index[i]].x * leftPoints[rng_index[i]].y;
//            A.at<double>(i, 4) = -1 * rightPoints[rng_index[i]].y * leftPoints[rng_index[i]].y;
//
//        }
//
//        cv::Mat y = cv::Mat::zeros(point_pick, 1, CV_64F);
//
//        for (int i = 0; i < point_pick; i++)
//        {
//            //A(i,:) = [pts2(1, i), pts2(2, i), 1, -1*pts2(1, i)*pts1(2, i), -1*pts2(2, i)*pts1(2, i)];
//            y.at<double>(i, 0) = leftPoints[rng_index[i]].y;
//        }
//
//        cv::Mat A_pinv = cv::Mat::zeros(A.size().width, A.size().height, CV_64F);
//        cv::invert(A, A_pinv, cv::DECOMP_SVD);
//
//        cv::Mat H_params = A_pinv * y;
//
//        double data[] = {1, 0, 0, H_params.at<double>(0, 0), H_params.at<double>(1, 0), H_params.at<double>(2, 0),
//                         H_params.at<double>(3, 0), H_params.at<double>(4, 0), 1};
//
//        cv::Mat temp_Hy(3, 3, CV_64F, data);
//
//        std::vector<cv::Point2f> rk_rng, rk_t;
//        rk_rng.reserve(point_pick);
//        rk_t.reserve(point_pick);
//
//        for (int i = 0; i < point_pick; i++)
//        {
//            rk_rng.push_back(rightPoints[rng_index[i]]);
//        }
//
//
//        cv::perspectiveTransform(rk_rng, rk_t, temp_Hy);
//        double error_sum = 0.f;
//        double threshold_size = 0.f;
//        for (int i = 0; i < point_pick; i++)
//        {
//            double error = abs(leftPoints[rng_index[i]].y - rk_t[i].y);
//            if (error <= 1)
//            {
//                error_sum = error_sum + error;
//                ++threshold_size;
//            }
//        }
//
//        if (threshold_size > 0.5f)
//        {
//            error_sum = error_sum / threshold_size;
//        }
//
//
//        //points2_t = htx(Hy, points2);
//        //errors = abs(points1(2, :) - points2_t(2, :));
//        //count = ones(size(errors));
//        //count(errors > threshold) = 0;
//        //align_rate = sum(count(:)) / double(size(count, 2));
//
//        if (error_sum > max_align_rate)
//        {
//            max_align_rate = error_sum;
//            Hy = temp_Hy;
//        }
//
//        if (max_align_rate > align_rate_earlystop)
//        {
//            break;
//        }
//
//    }
//
//    std::vector<int> mask;
//
//    cv::findHomography(leftPoints, rightPoints, CV_RANSAC, 3, mask, 10000, 0.9899999999999999911);
//    //cv::findFundamentalMat(leftPoints, rightPoints, CV_FM_RANSAC, 1, 0.9899999999999999911, mask);
//    std::vector<cv::Point2f> leftFilteredPoints, rightFilteredPoints;
//    leftFilteredPoints.reserve(mask.size());
//    rightFilteredPoints.reserve(mask.size());
//    for (int i = 0; i < mask.size(); ++i)
//    {
//        if (mask[i] != 0)
//        {
//            leftFilteredPoints.push_back(leftPoints[i]);
//            rightFilteredPoints.push_back(rightPoints[i]);
//        }
//    }
*/


	cv::Mat A = cv::Mat::zeros(static_cast<int>(leftFilteredPoints.size()), 5, CV_64F);

	for (int i = 0; i < leftFilteredPoints.size(); i++)
	{
		//A(i,:) = [pts2(1, i), pts2(2, i), 1, -1*pts2(1, i)*pts1(2, i), -1*pts2(2, i)*pts1(2, i)];

		A.at<double>(i, 0) = rightFilteredPoints[i].x;
		A.at<double>(i, 1) = rightFilteredPoints[i].y;
		A.at<double>(i, 2) = 1;
		A.at<double>(i, 3) = -1 * rightFilteredPoints[i].x * leftFilteredPoints[i].y;
		A.at<double>(i, 4) = -1 * rightFilteredPoints[i].y * leftFilteredPoints[i].y;
	}

	cv::Mat y = cv::Mat::zeros(static_cast<int>(leftFilteredPoints.size()), 1, CV_64F);

	for (int i = 0; i < leftFilteredPoints.size(); i++)
	{
		//A(i,:) = [pts2(1, i), pts2(2, i), 1, -1*pts2(1, i)*pts1(2, i), -1*pts2(2, i)*pts1(2, i)];
		y.at<double>(i, 0) = leftFilteredPoints[i].y;
	}

	cv::Mat A_pinv = cv::Mat::zeros(A.size().width, A.size().height, CV_64F);
	cv::invert(A, A_pinv, cv::DECOMP_SVD);

	cv::Mat H_params = A_pinv * y;

	double data4[] = {
		1, 0, 0, H_params.at<double>(0, 0), H_params.at<double>(1, 0), H_params.at<double>(2, 0),
		H_params.at<double>(3, 0), H_params.at<double>(4, 0), 1
	};

	cv::Mat temp_Hy(3, 3, CV_64F, data4);

	cv::Mat Hy = temp_Hy;

	auto imsize = originalSize;
	double data[] = {
		static_cast<double>(imsize.width) / 2, static_cast<double>(imsize.width),
		static_cast<double>(imsize.width) / 2, 0, 0, static_cast<double>(imsize.height),
		static_cast<double>(imsize.height),
		static_cast<double>(imsize.height) / 2
	};
	cv::Mat vertex(2, 4, CV_64F, data);


	//cv::Mat a = cv::Mat::zeros(1, 3, CV_64F);
	//cv::Mat b = cv::Mat::zeros(1, 3, CV_64F);
	//cv::Mat c = cv::Mat::zeros(1, 3, CV_64F);
	//cv::Mat d = cv::Mat::zeros(1, 3, CV_64F);

	//a.at<double>(0, 0) = (imsize.width - 1) / 2.f;
	//a.at<double>(0, 1) = 0;
	//a.at<double>(0, 2) = 1;

	//b.at<double>(0, 0) = imsize.width - 1;
	//b.at<double>(0, 1) = (imsize.height - 1) / 2.f;
	//b.at<double>(0, 2) = 1;

	//c.at<double>(0, 0) = (imsize.width - 1) / 2.f;
	//c.at<double>(0, 1) = imsize.height - 1;
	//c.at<double>(0, 2) = 1;

	//d.at<double>(0, 0) = 0;
	//d.at<double>(0, 1) = (imsize.height - 1) / 2.f;
	//d.at<double>(0, 2) = 1;

	cv::Point2d a((imsize.width - 1) / 2.f, 0);
	cv::Point2d b(imsize.width - 1, (imsize.height - 1) / 2.f);
	cv::Point2d c((imsize.width - 1) / 2.f, imsize.height - 1);
	cv::Point2d d(0, (imsize.height - 1) / 2.f);
	std::vector<cv::Point2d> p = {a, b, c, d};
	std::vector<cv::Point2d> pp;


	//a = a * Hy;
	//b = b * Hy;
	//c = c * Hy;
	//d = d * Hy;

	//cv::warpPerspective(a, a, Hy, {3, 1});
	//cv::warpPerspective(b, b, Hy, {3, 1});
	//cv::warpPerspective(c, c, Hy, {3, 1});
	//cv::warpPerspective(d, d, Hy, {3, 1});
	cv::perspectiveTransform(p, pp, Hy);


	// auto u = b - d;
	//auto v = a - c;
	auto u = pp[1] - pp[3];
	auto v = pp[0] - pp[2];

	double s_a = (imsize.height * imsize.height * u.y * u.y + imsize.width * imsize.width * v.y * v.y)
		/ (imsize.height * imsize.width * (u.y * v.x - u.x * v.y));

	double s_b = (imsize.height * imsize.height * u.x * u.y + imsize.width * imsize.width * v.x * v.y)
		/ (imsize.height * imsize.width * (u.x * v.y - u.y * v.x));

	double data1[] = {s_a, s_b, 0, 0, 1, 0, 0, 0, 1};

	cv::Mat Hs(3, 3, CV_64F, data1);

	cv::Mat H = Hs * Hy;

	//std::vector<cv::Point2f> k;
	//cv::perspectiveTransform(in_pr, k, H);

	std::vector<cv::Point2f> rk_t;
	cv::perspectiveTransform(rightFilteredPoints, rk_t, H);

	std::vector<cv::Point3f> rk_h;

	cv::convertPointsToHomogeneous(rightFilteredPoints, rk_h);
	for (int i = 0; i < rk_h.size(); i++)
	{
		cv::Vec3f a = rk_h[i];
		cv::Mat cc = cv::Mat::zeros(3, 1, CV_64F);
		cc.at<double>(0, 0) = a[0];
		cc.at<double>(1, 0) = a[1];
		cc.at<double>(2, 0) = a[2];
		cv::Mat xx(1, 1, cc.type());
		xx = H * cc;
		cv::Vec3d xyz(reinterpret_cast<double *>(xx.data));
		cv::Vec3f vvv = xyz;
		rk_h[i] = vvv;
	}

	std::vector<cv::Point2f> rk_tt;
	cv::convertPointsFromHomogeneous(rk_h, rk_tt);

	double error = 0.f;

	//for (int i = 0; i < lk.size(); ++i)
	//{
	//	double temp = abs(lk[i].y - rk_t[i].y);
	//	if (temp < 1)
	//	{
	//		temp = lk[i].x - rk_t[i].x;
	//		if (temp > error)
	//		{
	//			error = temp;
	//		}
	//	}
	//}

	error = 0;

	for (int i = 0; i < leftFilteredPoints.size(); ++i)
	{
		double temp = leftFilteredPoints[i].x - rk_t[i].x;
		if (temp > error)
		{
			error = temp;
		}
	}

	double data2[] = {1, 0, error, 0, 1, 0, 0, 0, 1};

	cv::Mat Hk(3, 3, CV_64F, data2);

	//H = Hk * Hs * Hy;

	//H = Hk * Hs * Hy;

	//cv::Mat r, rr, hk, hs, hy;

	//cv::warpPerspective(imageLight, r, H, imageLight.size());

	//cv::warpPerspective(imageLight, rr, Hs* Hy, imageLight.size());

	//cv::warpPerspective(imageLight, hk, Hk, imageLight.size());
	//cv::warpPerspective(imageLight, hs, Hs, imageLight.size());
	//cv::warpPerspective(imageLight, hy, Hy, imageLight.size());

	//rectImageLeft = imageLeft;
	//rectImageRight = rr;

	//cv::imwrite("s.png", rectImageLeft);
	//cv::imwrite("l.png", rectImageRight);


	homLeft = cv::Mat::eye(3, 3, CV_64F);

	//homRight = Hs * Hy;
	homRight = Hk * Hs * Hy;


	//this->calcError(leftFilteredPoints, rightFilteredPoints);

	//cv::imshow("rect", r);


	//cv::imwrite("rect.png", r);
	//cv::imwrite("rectrect.png", rr);

	//cv::waitKey(0);

	//for i = 1: size(pts1, 2)
	//	if abs(pts2_t(2, i) - pts1(2, i) < 1)
	//		errors = [errors, (pts1(1, i) - pts2_t(1, i))];
	//end
	//	end

	//double error = 0;

	//for (int i = 0; i < in_pl.size(); i++)
	//{
	//	//abs(pts2_t(2,i) - pts1(2,i)< 1)
	//	double temp = abs(k[i].x - in_pl[i].x);
	//	if (temp > error)
	//	{
	//		error = temp;
	//	}
	//}

	//double data2[] = { 1, 0, error, 0, 1, 0, 0, 0, 1 };

	//cv::Mat Hk(3, 3, CV_64F, data2);

	//H = Hk * Hs * Hy;

	//cv::Mat r;

	//cv::warpPerspective(imageLight, r, H, imageLight.size());

	//cv::imshow("rect", r);

	//cv::waitKey(0);
}

void Rectification::PoseEstimationRectification(const cv::Mat& leftCameraMatrix, const cv::Mat& rightCameraMatrix)
{
	//using kernelType = openMVG::robust::ACKernelAdaptor<openMVG::EightPointRelativePoseSolver,openMVG::fundamental::kernel::EpipolarDistanceError,>;
	//using kernelType = openMVG::essential::kernel::EssentialKernel<>

	// openMVG::Mat xL(2, leftPoints.size());
	// openMVG::Mat xR(2, leftPoints.size());
	// //openMVG::Mat x();
	//
	// for (int i = 0; i < leftPoints.size(); i++)
	// {
	// 	xL(0, i) = leftPoints[i].x;
	// 	xL(1, i) = leftPoints[i].y;
	// 	xR(0, i) = rightPoints[i].x;
	// 	xR(1, i) = rightPoints[i].y;
	// }
	//
	//
	// const openMVG::cameras::Pinhole_Intrinsic
	// 	camL(originalSize.width, originalSize.height, leftCameraMatrix.at<double>(0, 0),
	// 	     leftCameraMatrix.at<double>(0, 2), leftCameraMatrix.at<double>(1, 2)),
	// 	camR(originalSize.width, originalSize.height, rightCameraMatrix.at<double>(0, 0),
	// 	     rightCameraMatrix.at<double>(0, 2), rightCameraMatrix.at<double>(1, 2));
	//
	//
	// const std::pair<size_t, size_t> size_imaL(originalSize.width, originalSize.height);
	// const std::pair<size_t, size_t> size_imaR(originalSize.width, originalSize.height);
	//
	// std::cout << "essential" << std::endl;
	// openMVG::sfm::RelativePose_Info relativePose_info;
	// std::exception_ptr eptr;
	// try
	// {
	// 	if (!robustRelativePose(&camL, &camR, xL, xR, relativePose_info, size_imaL, size_imaR, 256))
	// 	{
	// 		std::cout << "problem" << std::endl;
	// 		std::cerr << " /!\\ Robust relative pose estimation failure."
	// 			<< std::endl;
	// 		return;
	// 	}
	// }
	// catch (...)
	// {
	// 	eptr = std::current_exception();
	// }
	// std::cout << "essential finished" << std::endl;


	
	cv::Mat F, E,E2, mask;

	//F = cv::findFundamentalMat(leftFilteredPoints, rightFilteredPoints, cv::FM_8POINT);

	//E2 = leftCameraMatrix.t() * F * rightCameraMatrix;

	std::vector<cv::Point2d> leftPoints_, rightPoints_;
	leftPoints_.reserve(leftPoints.size());
	rightPoints_.reserve(leftPoints.size());
	for(int i=0;i<leftPoints.size();i++)
	{
		double left_x = (leftPoints[i].x - leftCameraMatrix.at<double>(0,2))/leftCameraMatrix.at<double>(0,0);
		double left_y = (leftPoints[i].y - leftCameraMatrix.at<double>(1,2))/leftCameraMatrix.at<double>(1,1);
		cv::Point2d left(left_x, left_y);
		double right_x = (rightPoints[i].x - rightCameraMatrix.at<double>(0,2))/rightCameraMatrix.at<double>(0,0);
		double right_y = (rightPoints[i].y - rightCameraMatrix.at<double>(1,2))/rightCameraMatrix.at<double>(1,1);
		cv::Point2d right(right_x, right_y);

		leftPoints_.push_back(left);
		rightPoints_.push_back(right);
	}
	cv::Mat fakeCameraMatrix = cv::Mat::eye(3, 3, CV_64F);

	cv::Mat fakeE;
	double threshold = rejection_threshold;
	threshold /= ((leftCameraMatrix.at<double>(0, 0) + rightCameraMatrix.at<double>(0, 0)) / 2 + (leftCameraMatrix.at<double>(1, 1) + rightCameraMatrix.at<double>(1, 1)) / 2) / 2;

	//E = cv::findEssentialMat(leftPoints, rightPoints, leftCameraMatrix, cv::RANSAC, rejection_confidence, rejection_threshold, mask);
	fakeE = cv::findEssentialMat(leftPoints_, rightPoints_, fakeCameraMatrix, cv::RANSAC, rejection_confidence, threshold, mask);

	//cv::recoverPose(E, leftPoints, rightPoints, leftCameraMatrix, relativeRotation, relativeTranslation,mask);
	cv::recoverPose(fakeE, leftPoints_, rightPoints_, fakeCameraMatrix, relativeRotation, relativeTranslation,mask);
	//cv::recoverPose(E2, leftFilteredPoints, rightFilteredPoints, leftCameraMatrix, relativeRotation, relativeTranslation);
	
}


void Rectification::calcError(std::vector<cv::Point2f> left_points, std::vector<cv::Point2f> right_points)
{
	std::vector<cv::Point2f> left_points_projection, right_points_projection;

	cv::perspectiveTransform(left_points, left_points_projection, homLeft);
	cv::perspectiveTransform(right_points, right_points_projection, homRight);

	min_error = DBL_MAX;
	max_error = 0;
	avg_error = 0;

	for (int i = 0; i < left_points.size(); ++i)
	{
		double error = abs(left_points_projection[i].y - right_points_projection[i].y);
		if (min_error > error)
		{
			min_error = error;
		}
		if (max_error < error)
		{
			max_error = error;
		}
		avg_error += error;
	}

	avg_error /= left_points.size();

	std::cout << "min error = " << min_error << std::endl;
	std::cout << "max error = " << max_error << std::endl;
	std::cout << "avg error = " << avg_error << std::endl;
}


const cv::Mat& Rectification::getRectImageLeft() const
{
	return rectImageLeft;
}

const cv::Mat& Rectification::getRectImageMiddle() const
{
	return rectImageMiddle;
}

const cv::Mat& Rectification::getRectImageRight() const
{
	return rectImageRight;
}

double Rectification::getMin_error() const
{
	return min_error;
}

double Rectification::getMax_error() const
{
	return max_error;
}

double Rectification::getAvg_error() const
{
	return avg_error;
}
