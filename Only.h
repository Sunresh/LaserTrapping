#ifndef ONLY_H
#define ONLY_H

#include <iostream>
#include <deque>
#include <thread>
#include <iostream>
#include <nidaqmx.h>
#include <opencv2/opencv.hpp>
#include "Diamond.h"
using namespace std;

class Depositi {
private:
	cv::VideoCapture camm;
	Diamond diamond;
public:
	Depositi();
	~Depositi();
	int pztvolt;
	double thContrast;
	int timefordeposition;
	TaskHandle task1 = nullptr;
	TaskHandle task2 = nullptr;
	const char* dev0 = "Dev2/ao0";
	const char* dev1 = "Dev2/ao1";
	void camra();
	double calculateContrast(const cv::Mat& frame);
	void drawGraph(cv::Mat& frame, const std::deque<double>& graphValues);
	void drawGraphC(cv::Mat& frame, const std::deque<double>& graphValues);
	void allgraph(cv::Mat& frame, const std::deque<double>& graphValues);
	void allgraphc(cv::Mat& frame, const std::deque<double>& graphValues);
	void writeContrastToCSV(const std::string& filename, const std::vector<double>& contrastData);
	void dodeposition();
	void cAndD();
	std::string appName = "Laser Trapping stage feedback";
	std::string info = "LAEPD info";
	const std::string filename = "diamond.csv";
	cv::VideoCapture cam;
	int x1 = 315;
	int y1 = 250;
	cv::Point pt1 = cv::Point(x1, y1);
	int x2 = x1+9;
	int y2 = y1+9;
	cv::Point pt2 = cv::Point(x2, y2);
	const cv::Scalar& red = cv::Scalar(0, 0, 255);
	const cv::Scalar& white = cv::Scalar(255, 255, 255);
	const cv::Scalar& black = cv::Scalar(0, 0, 0);
	int roiw = x2 - x1;
	int roih = y2 - y1;
	int infowinw = 400;
	int infowinh = 400;
	int padding = 15;
	int infowinwp = infowinw - padding;
	int infowinhp = infowinh - padding;
	const int graphWidth = 400;
	const int graphHeight = 100;
	void drawText(cv::Mat& frame, const std::string& text, int x, int y, int fontSize, const cv::Scalar& color, int thickness);
	void drawRectangle(cv::Mat& frame, int x1, int y1, int x2, int y2, const cv::Scalar& color, int thickness);
	void app();
	void drawGraphLines(cv::Mat& graphArea, const std::deque<double>& graphValues);
	void drawCLines(cv::Mat& graphArea, const std::deque<double>& graphValues);
	void drawYAxisValues(cv::Mat& graphArea, int graphHeight, const cv::Scalar& color, const std::string& text);
	void drawXAxis(cv::Mat& graphArea, int graphHeight, const cv::Scalar& color);

	
};

#endif