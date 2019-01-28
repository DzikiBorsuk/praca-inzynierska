//
// Created by Piotr Meller on 14.11.18.
//

#ifndef PRACA_INZYNIERSKA_CALIBRATION_H
#define PRACA_INZYNIERSKA_CALIBRATION_H

#include <opencv2/core.hpp>
#include <opencv2/calib3d.hpp>

class Calibration
{
    cv::Mat cameraMatrix;
	cv::Mat cameraMatrixAfterUndistortion;
    cv::Mat distortionCoefficient;
    cv::Size patternSize;
	int patternType;
	float squareSize;
    std::vector<cv::Mat> imagesArray;
    //std::vector<cv::Mat> imagesArrayGray;
    std::vector<cv::Mat> undistortedImagesArray;
    std::vector<int> goodImages;
    std::vector<std::vector<cv::Point2f>> cornersArray;
    std::vector<std::vector<cv::Point3f>> realCornersArray;
    std::vector<cv::Mat> rvecArray;
    std::vector<cv::Mat> tvecArray;
    std::vector<double> reprojectionErrorsArray;
    double avgError;
    double avgError2;
    cv::Mat empty;


public:

    enum PatternType
    {
        CHESSBOARD,
        CIRCLES_GRID,
        CIRCLES_GRID_ASYMMETRIC
    };

    enum CalibOptions
    {

    };

    Calibration();
    Calibration(std::string filename);
    ~Calibration();

    void loadParams(const std::string &filename);
    void saveParams(const std::string &filename);

	const cv::Mat& getCameraMatrix() const;
	const cv::Mat& getDistortionCoefficient() const;
	const cv::Mat& getCameraMatrixAfterUndistortion() const;

    void loadImagesList(const std::vector<std::string> &image_path_list);

    void findCalibrationPoints(const cv::Size &pattern_size,
                               int patternType = PatternType::CHESSBOARD,
                               float squareSize = 1,
                               cv::InputArray mask = cv::noArray());

    void calibrateCamera(int flags = cv::CALIB_ZERO_TANGENT_DIST);

	void calibrateUndistortedCamera(int flags = cv::CALIB_ZERO_TANGENT_DIST);

    void calculateReprojectionError();

    void undistortCalibrationImages();

    void drawPattern();

    cv::Mat undistort(const cv::Mat &img);
    const std::vector<cv::Mat> &getImagesArray() const;
    const std::vector<int> &getGoodImages() const;
    const std::vector<double> &getReprojectionErrorsArray() const;
    const cv::Mat &getImage(int i) const;
    const cv::Mat &getUndistortedImage(int i) const;

    double getAvgError2() const;

private:
	void findPoints(const std::vector<cv::Mat> &images,const cv::Size &pattern_size,
		int patternType = PatternType::CHESSBOARD,
		float squareSize = 1);
};

#endif //PRACA_INZYNIERSKA_CALIBRATION_H

