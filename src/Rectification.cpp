//
// Created by Piotr Meller on 14.11.18.
//

#include "Rectification.h"
#include <opencv2/ccalib.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <vector>

Rectification::Rectification()
{
}


Rectification::~Rectification()
{
}



void Rectification::Hartley(const cv::Mat & left, std::vector<cv::Point2f> left_points, const cv::Mat & right, std::vector<cv::Point2f> right_points)
{

	cv::Mat fundMatrix = cv::findFundamentalMat(left_points, right_points, CV_FM_RANSAC, 0.5);
	cv::stereoRectifyUncalibrated(left_points, right_points, fundMatrix, right.size(), homLeft, homRight);

	//cv::warpPerspective(left, rect_left, homLeft, left.size());
	//cv::warpPerspective(right, rect_right, homRight, right.size());

	warp_image(left, right);
}

void Rectification::LoopZhang(const cv::Mat & left, std::vector<cv::Point2f> left_points, const cv::Mat & right, std::vector<cv::Point2f> right_points)
{

	cv::Mat fundMatrix = cv::findFundamentalMat(left_points, right_points, CV_FM_RANSAC, 0.5);
	cv::stereoRectifyUncalibrated(left_points, right_points, fundMatrix, right.size(), homLeft, homRight);

	//cv::warpPerspective(left, rect_left, homLeft, left.size());
	//cv::warpPerspective(right, rect_right, homRight, right.size());

	cv::Size imsize = left.size();

	cv::Point2d a((imsize.width - 1) / 2.f, 0);
	cv::Point2d b(imsize.width - 1, (imsize.height - 1) / 2.f);
	cv::Point2d c((imsize.width - 1) / 2.f, imsize.height - 1);
	cv::Point2d d(0, (imsize.height - 1) / 2.f);
	std::vector<cv::Point2d> p = { a,b,c,d };
	std::vector<cv::Point2d> pp;
	cv::perspectiveTransform(p, pp, homLeft);

	auto u = pp[1] - pp[3];
	auto v = pp[0] - pp[2];

	double s_a = (imsize.height * imsize.height * u.y * u.y + imsize.width * imsize.width * v.y * v.y) / (imsize.height * imsize.width *(u.y * v.x - u.x * v.y));

	double s_b = (imsize.height * imsize.height * u.x * u.y + imsize.width * imsize.width * v.x * v.y) / (imsize.height * imsize.width *(u.x * v.y - u.y * v.x));


	double data1[] = { s_a, s_b, 0, 0, 1, 0, 0, 0, 1 };

	cv::Mat Hs(3, 3, CV_64F, data1);

	homLeft = Hs * homLeft;

	//cv::warpPerspective(left, rect_left, homLeft, left.size());

	warp_image(left, right);
}

void Rectification::DSR(const cv::Mat & left, std::vector<cv::Point2f> left_points, const cv::Mat & right, std::vector<cv::Point2f> right_points)
{
	//cv::Mat fundMatrix = cv::findFundamentalMat(lk, rk, CV_FM_RANSAC, 1);
	//    cv::stereoRectifyUncalibrated(lk, rk, fundMatrix, right.size(), homLeft, homRight);
	//
	//    cv::warpPerspective(right, right, homRight, right.size());
	//    cv::warpPerspective(left, left, homLeft, left.size());
	//
	//
	//    cv::namedWindow("r", cv::WINDOW_NORMAL);
	//    cv::namedWindow("l", cv::WINDOW_NORMAL);
	//
	//    cv::imshow("r", right);
	//    cv::imshow("l", left);
	//    cv::resizeWindow("r", 1200, 900);
	//    cv::resizeWindow("l", 1200, 900);
	//
	//    cv::imwrite("rl.png",left);
	//    cv::imwrite("rr.png",right);
	//
	//    cv::waitKey(0);

	int ransac_iters = 1000;
	int point_pick = 200;

	cv::RNG random(cv::getCPUTickCount());

	std::vector<int> rng_index;
	rng_index.reserve(point_pick);

	double max_align_rate = 0.f;
	double align_rate_earlystop = 0.995;
	cv::Mat Hy = cv::Mat::eye(3, 3, CV_64F);
	int total_len = left_points.size();

	for (int i = 0; i < ransac_iters; i++)
	{
		rng_index.clear();

		int j = 0;
		while (j < point_pick)
		{
			int rand_i = random.next() % total_len;
			if (std::find(rng_index.begin(), rng_index.end(), rand_i) != rng_index.end()) {
				/* v contains x */
			}
			else {
				rng_index.push_back(rand_i);
				j++;
			}
		}

		cv::Mat A = cv::Mat::zeros(point_pick, 5, CV_64F);

		for (int i = 0; i < point_pick; i++)
		{
			//A(i,:) = [pts2(1, i), pts2(2, i), 1, -1*pts2(1, i)*pts1(2, i), -1*pts2(2, i)*pts1(2, i)];

			A.at<double>(i, 0) = right_points[rng_index[i]].x;
			A.at<double>(i, 1) = right_points[rng_index[i]].y;
			A.at<double>(i, 2) = 1;
			A.at<double>(i, 3) = -1 * right_points[rng_index[i]].x * left_points[rng_index[i]].y;
			A.at<double>(i, 4) = -1 * right_points[rng_index[i]].y * left_points[rng_index[i]].y;


		}

		cv::Mat y = cv::Mat::zeros(point_pick, 1, CV_64F);

		for (int i = 0; i < point_pick; i++)
		{
			//A(i,:) = [pts2(1, i), pts2(2, i), 1, -1*pts2(1, i)*pts1(2, i), -1*pts2(2, i)*pts1(2, i)];
			y.at<double>(i, 0) = left_points[rng_index[i]].y;
		}

		cv::Mat A_pinv = cv::Mat::zeros(A.size().width, A.size().height, CV_64F);
		cv::invert(A, A_pinv, cv::DECOMP_SVD);

		cv::Mat H_params = A_pinv * y;

		double data[] = { 1,0,0,H_params.at<double>(0, 0) ,H_params.at<double>(1, 0) ,H_params.at<double>(2, 0) ,H_params.at<double>(3, 0),H_params.at<double>(4, 0),1 };

		cv::Mat temp_Hy(3, 3, CV_64F, data);

		std::vector<cv::Point2f> rk_rng, rk_t;
		rk_rng.reserve(point_pick);
		rk_t.reserve(point_pick);

		for (int i = 0; i < point_pick; i++)
		{
			rk_rng.push_back(right_points[rng_index[i]]);
		}



		cv::perspectiveTransform(rk_rng, rk_t, temp_Hy);
		double error_sum = 0.f;
		double threshold_size = 0.f;
		for (int i = 0; i < point_pick; i++)
		{
			double error = abs(left_points[rng_index[i]].y - rk_t[i].y);
			if (error <= 1)
			{
				error_sum = error_sum + error;
				++threshold_size;
			}
		}

		if (threshold_size > 0.5f)
		{
			error_sum = error_sum / threshold_size;
		}


		//points2_t = htx(Hy, points2);
		//errors = abs(points1(2, :) - points2_t(2, :));
		//count = ones(size(errors));
		//count(errors > threshold) = 0;
		//align_rate = sum(count(:)) / double(size(count, 2));

		if (error_sum > max_align_rate)
		{
			max_align_rate = error_sum;
			Hy = temp_Hy;
		}

		if (max_align_rate > align_rate_earlystop)
		{
			break;
		}


	}

	std::vector<int> mask;

	cv::findHomography(left_points, right_points, CV_RANSAC, 3, mask, 10000, 0.9899999999999999911);
	//cv::findFundamentalMat(left_points, right_points, CV_FM_RANSAC, 1, 0.9899999999999999911, mask);
	std::vector<cv::Point2f> in_lk, in_rk;
	in_lk.reserve(mask.size());
	in_rk.reserve(mask.size());
	for (int i = 0; i < mask.size(); ++i)
	{
		if (mask[i] != 0)
		{
			in_lk.push_back(left_points[i]);
			in_rk.push_back(right_points[i]);
		}
	}

	cv::Mat A = cv::Mat::zeros(in_lk.size(), 5, CV_64F);

	for (int i = 0; i < in_lk.size(); i++)
	{
		//A(i,:) = [pts2(1, i), pts2(2, i), 1, -1*pts2(1, i)*pts1(2, i), -1*pts2(2, i)*pts1(2, i)];

		A.at<double>(i, 0) = in_rk[i].x;
		A.at<double>(i, 1) = in_rk[i].y;
		A.at<double>(i, 2) = 1;
		A.at<double>(i, 3) = -1 * in_rk[i].x * in_lk[i].y;
		A.at<double>(i, 4) = -1 * in_rk[i].y * in_lk[i].y;


	}

	cv::Mat y = cv::Mat::zeros(in_lk.size(), 1, CV_64F);

	for (int i = 0; i < in_lk.size(); i++)
	{
		//A(i,:) = [pts2(1, i), pts2(2, i), 1, -1*pts2(1, i)*pts1(2, i), -1*pts2(2, i)*pts1(2, i)];
		y.at<double>(i, 0) = in_lk[i].y;
	}

	cv::Mat A_pinv = cv::Mat::zeros(A.size().width, A.size().height, CV_64F);
	cv::invert(A, A_pinv, cv::DECOMP_SVD);

	cv::Mat H_params = A_pinv * y;

	double data4[] = { 1,0,0,H_params.at<double>(0, 0) ,H_params.at<double>(1, 0) ,H_params.at<double>(2, 0) ,H_params.at<double>(3, 0),H_params.at<double>(4, 0),1 };

	cv::Mat temp_Hy(3, 3, CV_64F, data4);

	Hy = temp_Hy;




	auto imsize = left.size();
	double data[] = { static_cast<double>(imsize.width) / 2, static_cast<double>(imsize.width),
					  static_cast<double>(imsize.width) / 2, 0, 0, static_cast<double>(imsize.height),
					  static_cast<double>(imsize.height),
					  static_cast<double>(imsize.height) / 2 };
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
	std::vector<cv::Point2d> p = { a,b,c,d };
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

	double s_a = (imsize.height * imsize.height * u.y * u.y + imsize.width * imsize.width * v.y * v.y) / (imsize.height * imsize.width *(u.y * v.x - u.x * v.y));

	double s_b = (imsize.height * imsize.height * u.x * u.y + imsize.width * imsize.width * v.x * v.y) / (imsize.height * imsize.width *(u.x * v.y - u.y * v.x));


	double data1[] = { s_a, s_b, 0, 0, 1, 0, 0, 0, 1 };

	cv::Mat Hs(3, 3, CV_64F, data1);

	cv::Mat H = Hs * Hy;

	//std::vector<cv::Point2f> k;
	//cv::perspectiveTransform(in_pr, k, H);

	std::vector<cv::Point2f> rk_t;
	cv::perspectiveTransform(in_rk, rk_t, H);

	std::vector<cv::Point3f> rk_h;

	cv::convertPointsToHomogeneous(in_rk, rk_h);
	for (int i = 0; i < rk_h.size(); i++)
	{
		cv::Vec3f a = rk_h[i];
		cv::Mat cc = cv::Mat::zeros(3, 1, CV_64F);
		cc.at<double>(0, 0) = a[0];
		cc.at<double>(1, 0) = a[1];
		cc.at<double>(2, 0) = a[2];
		cv::Mat xx(1, 1, cc.type());
		xx = H * cc;
		cv::Vec3d xyz((double*)xx.data);
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

	for (int i = 0; i < in_lk.size(); ++i)
	{
		double temp = in_lk[i].x - rk_t[i].x;
		if (temp > error)
		{
			error = temp;
		}

	}

	double data2[] = { 1, 0, error, 0, 1, 0, 0, 0, 1 };

	cv::Mat Hk(3, 3, CV_64F, data2);

	//H = Hk * Hs * Hy;

	H = Hk * Hs* Hy;

	//cv::Mat r, rr, hk, hs, hy;

	//cv::warpPerspective(right, r, H, right.size());

	//cv::warpPerspective(right, rr, Hs* Hy, right.size());

	//cv::warpPerspective(right, hk, Hk, right.size());
	//cv::warpPerspective(right, hs, Hs, right.size());
	//cv::warpPerspective(right, hy, Hy, right.size());

	//rect_left = left;
	//rect_right = rr;

	//cv::imwrite("s.png", rect_left);
	//cv::imwrite("l.png", rect_right);


	homLeft = cv::Mat::eye(3, 3, CV_64F);
	homRight = Hs * Hy;

	warp_image(left, right);

	//this->calcError(in_lk, in_rk);

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

	//cv::warpPerspective(right, r, H, right.size());

	//cv::imshow("rect", r);

	//cv::waitKey(0);

}

cv::Mat Rectification::getLeft()
{
	return rect_left.clone();
}

const cv::Mat Rectification::getRight()
{
	return rect_right.clone();
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

void Rectification::warp_image(const cv::Mat & left, const cv::Mat & right)
{
	originalSize = left.size();

	std::vector<cv::Point2f> left_points = { cv::Point2f(0,0),cv::Point2f(originalSize.width - 1,0),cv::Point2f(0,originalSize.height - 1),cv::Point2f(originalSize.width - 1,originalSize.height - 1) };
	std::vector<cv::Point2f> right_points = left_points;

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
	double data[] = { 1,0,-min_x,0,1,-min_y,0,0,1 };

	cv::Mat temp(3, 3, CV_64F, data);

	homLeftCorrection = temp * homLeft;
	homRightCorrection = temp * homRight;

	cv::Size size(max_x - min_x + 1, max_y - min_y + 1);

	cv::warpPerspective(left, rect_left, homLeftCorrection, size);
	cv::warpPerspective(right, rect_right, homRightCorrection, size);
}