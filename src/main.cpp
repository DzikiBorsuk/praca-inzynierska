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
	Stereo stereo;

    int last_i;
    std::cout<<"number of photo"<<std::endl;
    std::cin >> last_i;

    std::string path = "";
    for (int i = 1; i <= last_i; i++)
    {
        std::string temp = "";
        if (i < 10)
        {
            temp = temp + "0";
        }
        temp = temp + std::to_string(i);
        std::string load = path + temp + ".JPG";
        Mat src = imread(load);

        int width = src.size().width;
        int height = src.size().height;

        Rect r_left(0, 0, width / 2, height);
        Rect r_right(width / 2, 0, width / 2, height);


        Mat left_ = src(r_left);
        Mat right_ = src(r_right);
        Mat left, right;
        cv::resize(left_, left, { width, height });
        cv::resize(right_, right, { width, height });
        imwrite(path + "left/" + temp + ".png", left);
        imwrite(path + "right/" + temp + ".png", right);
    }


    //Stereo stereo("n_left.jpg","n_right.jpg");
//    Stereo stereo("2.JPG", "3.JPG", "out_camera_data.xml");
//
//    stereo.match_feautures();
//    stereo.rectifyImage();
//    stereo.computeDisp();

	stereo.loadLeftImage("2.JPG", "out_camera_data.xml");
	stereo.loadRightImage("1.JPG", "out_camera_data.xml");



	cv::imwrite("2.png", stereo.getLeft());
	cv::imwrite("1.png", stereo.getRight());


	//stereo.match_feautures();
	//stereo.rectifyImage();
	//stereo.computeDisp();

}