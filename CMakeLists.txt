cmake_minimum_required(VERSION 3.12)
project(praca_inzynierska)

set(CMAKE_CXX_STANDARD 17)
SET(OpenCV_DIR /installation/OpenCV-/share/OpenCV/)

find_package( OpenCV REQUIRED )

add_executable(praca_inzynierska main.cpp Stereo.cpp Stereo.h Rectification.cpp Rectification.h Calibration.cpp Calibration.h Disparity.cpp Disparity.h FeatureMatching.cpp FeatureMatching.h)


target_link_libraries(praca_inzynierska ${OpenCV_LIBS} )