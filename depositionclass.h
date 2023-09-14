#ifndef DEPOSITIONCLASS_H
#define DEPOSITIONCLASS_H
#include <iostream>
#include <opencv2/opencv.hpp>
#include <nidaqmx.h>
#include <sstream>
#include <fstream>
#include "cameraonly.h"
#include "preference.h"
using namespace std;

class Deposition : public CameraOnly {
private:
	cv::VideoCapture cam;
	TaskHandle task1 = nullptr;
	TaskHandle task2 = nullptr;
	const char* dev0 = "Dev2/ao0";
	const char* dev1 = "Dev2/ao1";
	double durationInSeconds = 2;
	double pztvolt = 0;
	double thContrast = 0;
	std::string commonPath = "C:/Users/nares/Desktop/allout/";
	const char allout[80] = "C:/Users/nares/Desktop/allout";
	const std::string filename = "preference.csv";
	const cv::Scalar& red = cv::Scalar(0, 0, 255);
	const cv::Scalar& white = cv::Scalar(255, 255, 255);
	const cv::Scalar& black = cv::Scalar(0, 0, 0);
	int roiw = x2 - x1;
	int roih = y2 - y1;
	Pref pr;
public:
	Deposition();
	~Deposition();
	void application();
	void Deposition::allgraph(cv::Mat& frame, const std::deque<double>& graphValues, double upperLimit);
	void Deposition::drawText(cv::Mat& frame, const std::string& text, int x, int y, double fontSize, const cv::Scalar& color, int thickness = 1);
	void Deposition::drawRectangle(cv::Mat& frame, int x1, int y1, int x2, int y2, const cv::Scalar& color, int thickness = 1);
	double Deposition::calculateContrast(const cv::Mat& frame);
	void Deposition::writeContrastToCSV(const std::string& filename, const std::vector<double>& contrastData, const std::string& xaxis, const std::string& yaxis);
	void Deposition::drawYAxisValues(cv::Mat& graphArea, const cv::Scalar& color, const double& text);
	void Deposition::drawXAxis(cv::Mat& graphArea, const cv::Scalar& color);
	void Deposition::DrawDashedLine(cv::Mat& img, cv::Point pt1, cv::Point pt2, cv::Scalar color, int thickness, std::string style, int gap);
};

#endif // DEPOSITIONCLASS_H
