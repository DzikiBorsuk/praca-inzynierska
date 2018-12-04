#cmake_minimum_required(VERSION 3.12)
cmake_minimum_required(VERSION 3.0)

project(praca_inzynierska)

set(CMAKE_CXX_STANDARD 17)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -pthread")
#SET(OpenCV_DIR /installation/OpenCV-/share/OpenCV/)

find_package( OpenCV REQUIRED )

file(GLOB_RECURSE SOURCE_FILES "src/*.cpp")
file(GLOB_RECURSE HEADER_FILES "src/*.h")

set (INCLUDE_DIRS "")
foreach(_headerFile ${HEADER_FILES})
    get_filename_component(_dir ${_headerFile} PATH)
    list (APPEND INCLUDE_DIRS ${_dir})
endforeach()
list(REMOVE_DUPLICATES INCLUDE_DIRS)

add_executable (praca_inzynierska ${SOURCE_FILES} src/StereoCalibration.cpp src/StereoCalibration.h)
target_include_directories(praca_inzynierska PRIVATE ${INCLUDE_DIRS})

#add_executable(praca_inzynierska cmake-build-debug/src/main.cpp cmake-build-debug/src/Stereo.cpp cmake-build-debug/src/Stereo.h cmake-build-debug/src/Rectification.cpp cmake-build-debug/src/Rectification.h cmake-build-debug/src/Calibration.cpp cmake-build-debug/src/Calibration.h cmake-build-debug/src/Disparity.cpp cmake-build-debug/src/Disparity.h cmake-build-debug/src/FeatureMatching.cpp cmake-build-debug/src/FeatureMatching.h)

target_link_libraries(praca_inzynierska ${OpenCV_LIBS} )



