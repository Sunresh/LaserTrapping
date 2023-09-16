#ifndef ONLY_H
#define ONLY_H
#include <iostream>
#include <opencv2/opencv.hpp>
#include <nidaqmx.h>
#include <sstream>
#include <fstream>
#include "preference.h"
using namespace std;

class Only {
private:
	cv::VideoCapture cam;
	Pref pr;
	TaskHandle task1 = nullptr;
	TaskHandle task2 = nullptr;

public:
	Only();
	~Only();
	void application();
	void allgraph(cv::Mat& frame, const std::deque<double>& graphValues, double upperLimit);
	void drawText(cv::Mat& frame, const std::string& text, int x, int y, double fontSize, const cv::Scalar& color, int thickness = 1);
	void drawRectangle(cv::Mat& frame, int x1, int y1, int x2, int y2, const cv::Scalar& color, int thickness = 1);
	double calculateContrast(const cv::Mat& frame);
	void writeContrastToCSV(const std::string& filename, const std::vector<double>& contrastData, const std::string& xaxis, const std::string& yaxis);
	void drawYAxisValues(cv::Mat& graphArea, const cv::Scalar& color, const double& text);
	void drawXAxis(cv::Mat& graphArea, const cv::Scalar& color);
	void DrawDashedLine(cv::Mat& img, cv::Point pt1, cv::Point pt2, cv::Scalar color, int thickness, std::string style, int gap);
};

#endif 
