//
// Created by Piotr Meller on 14.11.18.
//

#ifndef PRACA_INZYNIERSKA_RECTIFICATION_H
#define PRACA_INZYNIERSKA_RECTIFICATION_H

#include <opencv2/core.hpp>
#include <opencv2/ximgproc/disparity_filter.hpp>


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
		HARTLEY_UNDISTORTED,
		HARTLEY,
		DSR,
        POSE_ESTIMATION,
        NONE
	};

	//cv::Mat imageLeft, imageRight;
	cv::Mat leftRotation, rightRotation;

public: //private:

	cv::Mat rectImageLeft, rectImageMiddle, rectImageRight;
	cv::Mat rectImageLeftSizeCorrection, rectImageMiddleSizeCorrection, rectImageRightSizeCorrection;

	std::vector<cv::Point2f> leftPoints, middlePoints, rightPoints;
	cv::Mat inliers;
	std::vector<cv::Point2f> leftFilteredPoints, middleFilteredPoints, rightFilteredPoints;

	cv::Mat relativeTranslation, relativeRotation, Q;


	cv::Mat homLeft, homRight, homLeftCorrection, homRightCorrection;

	//cv::Mat leftRemap[2], middleRemap[2], rightRemap[2];

	cv::Size originalSize, newSize;

	double min_error, max_error, rms_error;

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


	void calcError();


	const cv::Mat& getRectImageLeft() const;
	const cv::Mat& getRectImageMiddle() const;
	const cv::Mat& getRectImageRight() const;
	double getMin_error() const;
	double getMax_error() const;
	double getRms_error() const;


private:

	/**
	Opencv implementation of hartley rectification
	*/
	void Hartley();

	/**
	Opencv implementation of hartley rectification + anti-shearing Loop-Zhang method
	*/
	void HartleyUndistorted();

	void DirectSelfRectification();

	void PoseEstimationRectification(const cv::Mat& leftCameraMatrix, const cv::Mat& rightCameraMatrix);


	int findEssentialMatrix(cv::InputArray _left_points, cv::InputArray _right_Points, cv::Mat& E, cv::Mat left_cam,
	                        cv::Mat right_cam)
	{
		cv::Mat l = _left_points.getMat(), r = _right_Points.getMat();

		cv::Mat left_points, right_points;

		l.convertTo(left_points, CV_64F);
		r.convertTo(right_points, CV_64F);

		// cv::Mat hom_l, hom_r;
		//
		// cv::convertPointsToHomogeneous(left_points, hom_l);
		// cv::convertPointsToHomogeneous(right_points, hom_r);
		//
		//
		//
		// hom_l = left_cam * hom_l;
		// hom_r = right_cam * hom_r;
		//
		// cv::convertPointsFromHomogeneous(hom_l, left_points);
		// cv::convertPointsFromHomogeneous(hom_r, right_points);


		// const cv::Point2d* m1 = left_points.ptr<cv::Point2d>();
		// const cv::Point2d* m2 = right_points.ptr<cv::Point2d>();

		std::vector<cv::Point2d> m1;
		std::vector<cv::Point2d> m2;

		std::vector<cv::Point3d> m1hom;
		std::vector<cv::Point3d> m2hom;


		int i, count = l.checkVector(2);

		m1.assign(left_points.ptr<cv::Point2d>(), left_points.ptr<cv::Point2d>() + count);
		m2.assign(right_points.ptr<cv::Point2d>(), right_points.ptr<cv::Point2d>() + count);

		cv::convertPointsToHomogeneous(m1, m1hom);
		cv::convertPointsToHomogeneous(m2, m2hom);

		cv::Matx<double, 9, 9> A;

		cv::Mat m1hom_mat(count, 3, CV_64F, m1hom.data());
		cv::Mat m2hom_mat(count, 3, CV_64F, m2hom.data());

		m1hom_mat = m1hom_mat.t();
		m2hom_mat = m2hom_mat.t();

		m1hom_mat = left_cam.inv() * m1hom_mat;
		m2hom_mat = right_cam.inv() * m2hom_mat;


		m1hom_mat = m1hom_mat.t();
		m2hom_mat = m2hom_mat.t();

		m1hom.assign(reinterpret_cast<cv::Point3d*>(m1hom_mat.data),
		             reinterpret_cast<cv::Point3d*>(m1hom_mat.data) + m1hom_mat.rows);
		m2hom.assign(reinterpret_cast<cv::Point3d*>(m2hom_mat.data),
		             reinterpret_cast<cv::Point3d*>(m2hom_mat.data) + m2hom_mat.rows);


		cv::convertPointsFromHomogeneous(m1hom, m1);
		cv::convertPointsFromHomogeneous(m2hom, m2);



		for (i = 0; i < count; i++)
		{
			//cv::Mat m1homo(count,2,CV_64F,m1)
			// double x1 = (m1[i].x - left_cam.at<double>(0, 2)) / left_cam.at<double>(0, 0);
			// double y1 = (m1[i].y - left_cam.at<double>(1, 2)) / left_cam.at<double>(1, 1);
			// double x2 = (m2[i].x - right_cam.at<double>(0, 2)) / right_cam.at<double>(0, 0);
			// double y2 = (m2[i].y - right_cam.at<double>(1, 2)) / right_cam.at<double>(1, 1);

			double x1 = m1[i].x;
			double y1 = m1[i].y;
			double x2 = m2[i].x;
			double y2 = m2[i].y;

			cv::Vec<double, 9> r(x2 * x1, x2 * y1, x2, y2 * x1, y2 * y1, y2, x1, y1, 1);
			A += r * r.t();
		}

		cv::Vec<double, 9> W;
		cv::Matx<double, 9, 9> V;

		eigen(A, W, V);


		cv::Matx33d F0(V.val + 9 * 8);



		cv::Vec3d w;
		cv::Matx33d U;
		cv::Matx33d Vt;

		cv::SVD::compute(F0, w, U, Vt);
		w[2] = 0.;

		F0 = U * cv::Matx33d::diag(w) * Vt;

		cv::Mat(F0).copyTo(E);

		return 1;
	}
};

#endif //PRACA_INZYNIERSKA_RECTIFICATION_H
