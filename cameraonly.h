#ifndef CAMERAONLY_H
#define CAMERAONLY_H
#include <iostream>
#include <opencv2/opencv.hpp>
#include <nidaqmx.h>
#include <sstream>
#include <fstream>
using namespace std;

class CameraOnly {
public:
	cv::VideoCapture cam;
	double durationInSeconds = 2;
	double pztvolt = 0;
	double thContrast = 0;
	const std::string filename = "preference.csv";
	int x1 = 359;
	int y1 = 235;
	cv::Point pt1 = cv::Point(x1, y1);
	int x2 = x1 + 7;
	int y2 = y1 + 7;
	cv::Point pt2 = cv::Point(x2, y2);
	const cv::Scalar& red = cv::Scalar(0, 0, 255);
	const cv::Scalar& white = cv::Scalar(255, 255, 255);
	const cv::Scalar& black = cv::Scalar(0, 0, 0);
	int roiw = x2 - x1;
	int roih = y2 - y1;
	CameraOnly();
	~CameraOnly();
	void application();
};

#endif // CAMERAONLY_H
