//
// Created by Piotr Meller on 06.01.19.
//

#ifndef IMAGE_STRUCTURE_H
#define IMAGE_STRUCTURE_H

#include <opencv2/core.hpp>
#include <opencv2/videostab/ring_buffer.hpp>

struct ImageStructure
{
	cv::Mat image;
	cv::Mat cameraMatrix;
	cv::Mat distortionCoefficient;

	explicit ImageStructure()
	{
		cameraMatrix = cv::Mat::eye(3, 3, CV_64F);
		distortionCoefficient = cv::Mat::zeros(5, 1, CV_64F);
	}

	ImageStructure(const ImageStructure& image_structure)
	{
		this->image = image_structure.image.clone();
		this->cameraMatrix = image_structure.cameraMatrix.clone();
		this->distortionCoefficient = image_structure.distortionCoefficient.clone();
	}

	ImageStructure& operator=(const ImageStructure& image_structure)
	{
		if (this != &image_structure)
		{
			this->image = image_structure.image.clone();
			this->cameraMatrix = image_structure.cameraMatrix.clone();
			this->distortionCoefficient = image_structure.distortionCoefficient.clone();
		}
		return *this;
	}

	void resize(const cv::Size& newSize)
	{
		double data[] = {1.0*newSize.width / image.size().width, 0, 0, 0, 1.0*newSize.height / image.size().height, 0, 0, 0, 1};
		cv::Mat transformation(3, 3, CV_64F, data);
		cameraMatrix = transformation * cameraMatrix;
		cv::Mat temp = image.clone();
		cv::resize(temp, image, newSize);
	}
};

#endif //IMAGE_STRUCTURE_H
