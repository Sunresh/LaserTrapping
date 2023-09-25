#ifndef DEPOSITIONCLASS_H
#define DEPOSITIONCLASS_H
#include <iostream>
#include <opencv2/opencv.hpp>
#include <nidaqmx.h>
#include <sstream>
#include <fstream>
#include "preference.h"
using namespace std;

class Deposition {
private:
	cv::VideoCapture cam;
	TaskHandle task1 = nullptr;
	TaskHandle task2 = nullptr;
	double contrast = 1.0;
	int fwidth;
	int fheight;
public:
	Pref pr;
	Deposition();
	~Deposition();
	void camera();
	void application();
	void Deposition::allgraph(cv::Mat& frame, const std::deque<double>& graphValues, double upperLimit);
	void Deposition::drawText(cv::Mat& frame, const std::string& text, int x, int y, double fontSize, const cv::Scalar& color, int thickness = 1);
	void Deposition::drawRectangle(cv::Mat& frame, int x1, int y1, int x2, int y2, const cv::Scalar& color, int thickness = 1);
	double Deposition::calculateContrast(const cv::Mat& frame);
	double Deposition::stdev(const cv::Mat& frame);
	double Deposition::mean(const cv::Mat& frame);
	void Deposition::writeContrastToCSV(const std::string& filename, const std::vector<double>& contrastData, const std::string& xaxis, const std::string& yaxis);
	void Deposition::drawYAxisValues(cv::Mat& graphArea, const cv::Scalar& color, const double& text);
	void Deposition::drawXAxis(cv::Mat& graphArea, const cv::Scalar& color);
	void Deposition::DrawDashedLine(cv::Mat& img, cv::Point pt1, cv::Point pt2, cv::Scalar color, int thickness, std::string style, int gap);

	void Deposition::laserspot(cv::Mat& dframe, double elapsedTime, cv::Mat& fullscreenimage);
	int numSteps = TTIME * 100;
	
	double etime = 0;
	bool updated = false;
	bool isIncrease = true;
	double voltage = 0.0;
	double electrophoretic = 0.0;
	cv::Mat frame, dframe, grayColorRect, gRect;
	std::vector<double> contrastData, grphValues;
	std::deque<double> pixData, grphVa, lla;
	int timedelay = 0;

	std::string exportfile;
	

	void Deposition::setfwidth(int windowwidth);
	int Deposition::getfwidth() const;
	void Deposition::setfheight(int windowHeight);
	int Deposition::getfheight() const;
	void Deposition::setOutputFileName(std::string filename);
	std::string Deposition::getOutputFileName() const;

	double elapsedTime;
	void Deposition::getelapsedTime(std::chrono::time_point<std::chrono::high_resolution_clock> startTime);
	std::string Deposition::double2string(const double& value, const std::string& stri);
};

#endif // DEPOSITIONCLASS_H
