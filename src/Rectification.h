//
// Created by Piotr Meller on 14.11.18.
//

#ifndef PRACA_INZYNIERSKA_RECTIFICATION_H
#define PRACA_INZYNIERSKA_RECTIFICATION_H

#include <opencv2/core.hpp>

class Rectification
{
    cv::Mat imageLeft, imageRight;

    cv::Mat rectImageLeft;
    cv::Mat rectImageRight;

    std::vector<cv::Point2f> leftPoints, rightPoints;
    std::vector<cv::Point2f> leftFilteredPoints, rightFilteredPoints;

    cv::Mat homLeft, homRight, homLeftCorrection, homRightCorrection;

    cv::Size originalSize;

    double min_error, max_error, avg_error;

public:
    Rectification();
    ~Rectification();

    enum RejectionMethod
    {
        RANSAC,
        LMEDS
    };

    enum RectificationMethod
    {
        LOOP_ZHANG,
        HARTLEY,
        DSR
    };

    void setLeftImage(const cv::Mat &left);
    void setRightImage(const cv::Mat &right);

    void setPoints(const std::vector<cv::Point2f> &left_points,
                   const std::vector<cv::Point2f> &right_points);

    void rejectOutliers(int method, double threshold, double confidence, int iterations);

    void rectifyImages(int method);


    void calcError(std::vector<cv::Point2f> left_points, std::vector<cv::Point2f> right_points);

    const cv::Mat &getImageLeft() const;
    const cv::Mat &getImageRight() const;
    const cv::Mat &getRectImageLeft() const;
    const cv::Mat &getRectImageRight() const;
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
    void warp_image();

};

#endif //PRACA_INZYNIERSKA_RECTIFICATION_H
