#ifndef DEPOSITIONCLASS_H
#define DEPOSITIONCLASS_H
#include <iostream>
#include <opencv2/opencv.hpp>
#include <nidaqmx.h>
#include <sstream>
#include <fstream>
#include "preference.h"
#include "brightnessclass.h"
#include "daqanalog.h"

using namespace std;

class Deposition {
private:
	cv::VideoCapture cam;
	MyDaq mydaq;
	TaskHandle task1 = nullptr;
	TaskHandle task2 = nullptr;
	int fwidth;
	int fheight;
	bool isFeedbackstart;
	
public:
	Pref pr;
	Deposition();
	~Deposition();
	void camera();
	void application();
	void Deposition::allgraph(cv::Mat& frame, std::deque<double>& graphValues, double upperLimit, const std::string& yxix);
	void Deposition::drawText(cv::Mat& frame, const std::string& text, int x, int y, double fontSize, const cv::Scalar& color, int thickness = 1);
	void Deposition::drawRectangle(cv::Mat& frame, int x1, int y1, int x2, int y2, const cv::Scalar& color, int thickness = 1);
	void Deposition::writeContrastToCSV(const std::string& filename, const std::vector<double>& contrastData, const std::vector<double>& data3, const std::deque<double>& data4, const std::string& xaxis, const std::string& yaxis, const std::string& name3);
	void Deposition::drawYAxisValues(cv::Mat& graphArea, const cv::Scalar& color, const double& text, const std::string& yaxis);
	void Deposition::drawXAxis(cv::Mat& graphArea, const cv::Scalar& color);
	void Deposition::DrawDashedLine(cv::Mat& img, cv::Point pt1, cv::Point pt2, cv::Scalar color, int thickness, std::string style, int gap);
	static void Deposition::onMouse(int event, int x, int y, int flags, void* userdata);
	void Deposition::laserspot(cv::Mat& dframe, double elapsedTime, cv::Mat& fullscreenimage);
	double etime = 0;
	double averagediff, cBR, cHT;
	bool isComplete = false;
	bool isRedeposition = false;
	bool isWithoutredeposition = true;
	double voltage = 0.0;
	cv::Mat frame, dframe, grayColorRect, gRect;
	std::vector<double> contrastData, grphValues;
	std::deque<double> pixData, grphVa, lla,feed_deque;
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
	double Deposition::stdev(std::deque<double> pixData);
	void Deposition::wToCSV(const std::string& filename, const std::string& name);
	void Deposition::copyFrame(cv::Mat& frame, cv::Mat& screenImage, int x, int y, int x2, int y2);
	void Deposition::setcurrentBrightness(cv::Mat& frame);
	double Deposition::getcurrentBrightness();
	void Deposition::setcurrentHeight(double voltage);
	double Deposition::getcurrentHeight();
	bool isCameraOnly;
	double Deposition::feedbackSD();
	int Deposition::numSteps();
	double epv = 0.0;
	void setEV(double ephv = 2.2) {
		epv = ephv;
	}
	double getEV() {
		return epv;
	}

};

#endif // DEPOSITIONCLASS_H
