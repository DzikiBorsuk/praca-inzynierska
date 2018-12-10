//
// Created by Piotr Meller on 15.10.18.
//

#define OPENCV_TRAITS_ENABLE_DEPRECATE
#include "Stereo.h"


int main()
{
    //Stereo stereo("n_left.jpg","n_right.jpg");
    Stereo stereo("2.JPG", "3.JPG", "out_camera_data.xml");

    stereo.match_feautures();
    stereo.rectifyImage();
    stereo.computeDisp();

}