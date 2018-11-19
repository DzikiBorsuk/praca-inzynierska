//
// Created by Piotr Meller on 14.11.18.
//

#ifndef PRACA_INZYNIERSKA_DISPARITY_H
#define PRACA_INZYNIERSKA_DISPARITY_H

#include <opencv2/core.hpp>
#include <opencv2/ximgproc.hpp>

class Disparity
{
    cv::Mat left_disp, right_disp, filtered_disp;

    //cv::Ptr<cv::StereoSGBM> left_matcher, right_matcher;
    cv::Ptr<cv::StereoSGBM> left_matcher;
    cv::Ptr<cv::StereoMatcher> right_matcher;

    int minDisparity, maxDisparity, blockSize, P1, P2, disp12MaxDiff, preFilterCap, uniquenessRatio,
        speckleWindowSize, speckleRange, mode;

    void setMatcher()
    {
        left_matcher->setMinDisparity(minDisparity);
        left_matcher->setNumDisparities(maxDisparity - minDisparity);
        left_matcher->setBlockSize(blockSize);
        left_matcher->setP1(P1);
        left_matcher->setP2(P2);
        left_matcher->setDisp12MaxDiff(disp12MaxDiff);
        left_matcher->setPreFilterCap(preFilterCap);
        left_matcher->setUniquenessRatio(uniquenessRatio);
        left_matcher->setSpeckleWindowSize(speckleWindowSize);
        left_matcher->setSpeckleRange(speckleRange);
        left_matcher->setMode(mode);

        right_matcher = cv::ximgproc::createRightMatcher(left_matcher);
    };

public:

    enum
    {
        MODE_SGBM,
        MODE_HH,
        MODE_SGBM_3WAY,
        MODE_HH4,
    };

    Disparity();
    ~Disparity();

    void initSGBM(int minDisparity = 0,
                  int numDisparities = 16,
                  int blockSize = 3,
                  int P1 = 0,
                  int P2 = 0,
                  int disp12MaxDiff = 0,
                  int preFilterCap = 0,
                  int uniquenessRatio = 0,
                  int speckleWindowSize = 0,
                  int speckleRange = 0,
                  int mode = MODE_SGBM);

    void SGBM(const cv::Mat &left, const cv::Mat &right);

    int getMinDisparity() const
    {
        return minDisparity;
    }
    void setMinDisparity(int minDisparity)
    {
        this->minDisparity = minDisparity;
        setMatcher();
    }
    int getMaxDisparities() const
    {
        return maxDisparity;
    }
    void setMaxDisparities(int numDisparities)
    {
        this->maxDisparity = numDisparities;
        setMatcher();
    }
    int getBlockSize() const
    {
        return blockSize;
    }
    void setBlockSize(int blockSize)
    {
        this->blockSize = blockSize;
        setMatcher();
    }
    int getP1() const
    {
        return P1;
    }
    void setP1(int P1)
    {
        this->P1 = P1;
        setMatcher();
    }
    int getP2() const
    {
        return P2;
    }
    void setP2(int P2)
    {
        this->P2 = P2;
        setMatcher();
    }
    int getDisp12MaxDiff() const
    {
        return disp12MaxDiff;
    }
    void setDisp12MaxDiff(int disp12MaxDiff)
    {
        this->disp12MaxDiff = disp12MaxDiff;
        setMatcher();
    }
    int getPreFilterCap() const
    {
        return preFilterCap;
    }
    void setPreFilterCap(int preFilterCap)
    {
        this->preFilterCap = preFilterCap;
        setMatcher();
    }
    int getUniquenessRatio() const
    {
        return uniquenessRatio;
    }
    void setUniquenessRatio(int uniquenessRatio)
    {
        this->uniquenessRatio = uniquenessRatio;
        setMatcher();
    }
    int getSpeckleWindowSize() const
    {
        return speckleWindowSize;
    }
    void setSpeckleWindowSize(int speckleWindowSize)
    {
        this->speckleWindowSize = speckleWindowSize;
        setMatcher();
    }
    int getSpeckleRange() const
    {
        return speckleRange;
    }
    void setSpeckleRange(int speckleRange)
    {
        this->speckleRange = speckleRange;
        setMatcher();
    }
    int getMode() const
    {
        return mode;
    }
    void setMode(int mode)
    {
        this->mode = mode;
        setMatcher();
    }

};

#endif //PRACA_INZYNIERSKA_DISPARITY_H

