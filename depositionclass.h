#pragma once

#include <iostream>
#include <opencv2/opencv.hpp>
#include <nidaqmx.h>
#include <sstream>
#include <fstream>
#include "preference.h"
#include "brightnessclass.h"
#include "daqanalog.h"
#include <windows.h>

using namespace std;

class Deposition {
private:
	cv::VideoCapture cam;
	MyDaq mydaq;
	TaskHandle task1 = nullptr;
	TaskHandle task2 = nullptr;
	int fwidth;
	int fheight;
	double etime = 0;
	double averagediff, cBR, cHT;
	bool isComplete = false;
	bool isRedeposition = false;
	bool isWithoutredeposition = true;
	double voltage = 0.0;
	cv::Mat frame, dframe, grayColorRect, gRect;
	std::vector<double> contrastData, pztValues;  /*sdValues*/
	int timedelay = 0;
	std::string exportfile;
	double elapsedTime;
	double padding = 10.0;
	bool isCameraOnly = true;
	char key;
	std::string WindowName = "LaserTrap";
public:
	Pref pr;
	Deposition::Deposition() :
		fwidth(GetSystemMetrics(SM_CXSCREEN) - 10), 
		fheight(GetSystemMetrics(SM_CYSCREEN) - 90), 
		exportfile("Default"),
		elapsedTime(),
		averagediff(0), 
		cBR(0), cHT(0)
	{
		mydaq.start(nullptr, "Dev2/ao0", 0);
		mydaq.start(nullptr, "Dev2/ao1", 0);
		//mydaq.digitalOut(nullptr, "Dev2/port0/line0", 1);
		//cam.open(pr.getCameraId());
		cam.open("C:\\Users\\nares\\Desktop\\LAB\\movie\\open\\five.mp4");
		double etime = 0;
		double voltage = 0.0;
		cv::Mat frame, dframe, grayColorRect, gRect;
		int timedelay = 0;
	}
	int numSteps() {
		return  pr.getDurationTime() * 100;
	}
	void setfwidth(int windowwidth) {
		if (fwidth > 0)
			fwidth = windowwidth;
		else
			throw std::invalid_argument("Height must be greater than 0.");
	}
	int getfwidth() const {
		return fwidth;
	}
	void setfheight(int windowHeight) {
		if (fheight > 0)
			fheight = windowHeight;
		else
			throw std::invalid_argument("Height must be greater than 0.");
	}
	int getfheight() const {
		return fheight;
	}

	void setOutputFileName(std::string filename) {
			auto now = std::chrono::system_clock::now();
			auto time_t_now = std::chrono::system_clock::to_time_t(now);
			struct tm timeinfo;
	#ifdef _MSC_VER
			localtime_s(&timeinfo, &time_t_now);
	#else
			localtime_r(&time_t_now, &timeinfo);
	#endif

			std::ostringstream oss;
			oss << std::put_time(&timeinfo, "%Y%m%d_%H%M_%S");
			std::ostringstream folder;
			folder << std::put_time(&timeinfo, "%Y%m%d");

			// Assuming pr is an instance of some class with getCommonPath() method
			std::string commonPath = pr.getCommonPath();

			// Create the folder inside the common path
			std::string folderPath = commonPath + "/" + folder.str();
			// Code to create the folder if it doesn't exist
			CreateDirectory(folderPath.c_str(), NULL) != 0;
			// Set the output file name using the common path, folder, and filename
			exportfile = folder.str()+"/" + oss.str();
	}

	std::string Deposition::getOutputFileName() const {
		return exportfile;
	}

	void Deposition::setcurrentBrightness(cv::Mat& frame) {
		BrightnessClass bri(frame);
		double contrast = bri.differencesOf();
		cBR = contrast;
	}

	double Deposition::getcurrentBrightness() {
		return cBR;
	}
	void Deposition::setcurrentHeight(double voltage) {
		double height = 6 * voltage;
		cHT = height;
	}
	double Deposition::getcurrentHeight() {
		return cHT;
	}
	void Deposition::getelapsedTime(std::chrono::time_point<std::chrono::high_resolution_clock> startTime) {
		auto currentTime = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> elapsed = currentTime - startTime;
		elapsedTime = elapsed.count();
	}
	std::string Deposition::double2string(const double& value, const std::string& stri) {
		std::stringstream sis;
		sis << stri << std::fixed << std::setprecision(3) << value;
		std::string thrr = sis.str();
		return thrr;
	}

	Deposition::~Deposition() {
		cam.release();
		cv::destroyAllWindows();
		cout << "End" << endl;
	}
	static void ButtonClick(int event, int x, int y, int flags, void* param) {
		auto self = static_cast<Deposition*>(param);
		if (event == cv::EVENT_LBUTTONDOWN) {
			if (x > self->padding && x < (self->fwidth * 0.10)- self->padding) {
				if (y > self->padding && y < (self->fheight * 0.10- self->padding)) {
					self->isCameraOnly = false;//start
				}
				else if (y > (self->fheight * 0.10) + self->padding && y < (self->fheight * 0.20) - self->padding) {
					self->isCameraOnly = true;//pause
				}
				else if (y > (self->fheight * 0.20) + self->padding && y < (self->fheight * 0.30 - self->padding)) {
					self->isComplete = true;//stop
				}
				else {
					std::cout << "\n(" << x << ", " << y << ")\n" << std::endl;
				}
			}
			else {
				std::cout << "\n(" << x << ", " << y << ")\n" << std::endl;
			}
		}
	}

	void application() {
		try {
			if (!cam.isOpened()) {
				return;
			}
			DAQmxCreateTask("", &task1);
			DAQmxCreateTask("", &task2);
			DAQmxCreateAOVoltageChan(task1, pr.deva1(), "ao_channel", 0.0, 5.0, DAQmx_Val_Volts, nullptr);
			DAQmxCreateAOVoltageChan(task2, pr.deva0(), "ao_channel", 0.0, 5.0, DAQmx_Val_Volts, nullptr);
			DAQmxCfgSampClkTiming(task1, "", 10.0, DAQmx_Val_Rising, DAQmx_Val_ContSamps, 1);
			DAQmxCfgSampClkTiming(task2, "", 10.0, DAQmx_Val_Rising, DAQmx_Val_ContSamps, 1);

			auto startTime = std::chrono::high_resolution_clock::now();
			cv::Mat fullScreenImage(fheight, fwidth, CV_8UC3, pr.uBGR(255, 255, 255));
			setOutputFileName(pr.getCommonPath());
			Deposition dep;
			cv::namedWindow(WindowName);
			cv::setMouseCallback(WindowName, &Deposition::ButtonClick, &dep);
			while (true) {
				cam >> dframe;
				if (dframe.empty()) {
					break;
				}
				getelapsedTime(startTime);
				laserspot(dframe, elapsedTime, fullScreenImage);
				SchmittTrigger schmittTrigger(pr.getUpperTh(), pr.getLowerTh()); // Set upper and lower thresholds
				bool output = schmittTrigger.processInput(getcurrentBrightness());
				Memory mm;

				contrastData.push_back(getcurrentBrightness());
				pztValues.push_back(voltage);
				if (!dep.isCameraOnly) {
					if (!dep.isComplete) {
						
						if (voltage < 0) {
							voltage = 0.0;
							timedelay = 0.0;
						}
						if (output && isWithoutredeposition && !isRedeposition) {
							if (voltage < (0.04*pr.maxVolt())) {
								voltage += (0.0002 * pr.maxVolt());
							}
							else {
								voltage += pr.maxVolt() / (numSteps());
							}
							setEV();
						}
						if (output && isRedeposition) {
							voltage += pr.maxVolt() / (numSteps() + timedelay);
							setEV();
						}
						if (!output) {
							timedelay += 1;
							voltage -= pr.maxVolt() / (numSteps() * 0.25);
							setEV();
							isRedeposition = true;
							isWithoutredeposition = false;
						}
						if (voltage >= pr.maxVolt() && !dep.isComplete) {
							etime = elapsedTime;
							isComplete = true;
							voltage -= pr.maxVolt() / numSteps();
							setEV(0);
							mydaq.digitalOut(nullptr, "Dev2/port0/line0", 0);
						}
						writeContrastToCSV(pr.getCommonPath() + exportfile + ".csv", contrastData, pztValues, /*sdValues,*/ "BD", "PZT volt");
					}
					if (dep.isComplete) {
						voltage -= pr.maxVolt() / (numSteps() * 0.2);
						setEV(0);
						if (voltage < 0) {
							voltage = 0;
							setEV(0);
							cv::imwrite(pr.getCommonPath() + exportfile + ".jpg", fullScreenImage);
							cv::destroyWindow(WindowName);
							break;
						}
					}
					double ev = getEV();
					DAQmxWriteAnalogF64(task2, 1, true, 10.0, DAQmx_Val_GroupByChannel, &ev, nullptr, nullptr);
					DAQmxWriteAnalogF64(task1, 1, true, 10.0, DAQmx_Val_GroupByChannel, &voltage, nullptr, nullptr);
					setcurrentHeight(voltage);
					mm.storeValue(voltage);
					if (key == '9') {
						isCameraOnly = true;
					}
				}
				if (key == '5') {
					isCameraOnly = false;
				}
				if (key == 'rr') {
					isCameraOnly = false;
				}

				cv::imshow(WindowName, fullScreenImage);
				cv::moveWindow(WindowName, 0, 0);
				key = cv::waitKey(1);
				if (key == 'q' || key == ' ') {
					isComplete = true;
					voltage -= pr.maxVolt() / (numSteps() * 0.1);
					setEV(0);
					mydaq.start(nullptr, "Dev2/ao0", 0);
					mydaq.digitalOut(nullptr, "Dev2/port0/line0", 0);
					if (voltage < 0) {
						cam.release();
						cv::destroyAllWindows();
						break;
					}
				}
			}
			DAQmxClearTask(task1);
			DAQmxClearTask(task2);
		}
		catch (const std::exception& e) {
			std::cerr << "\nAn exception occurred by Naresh: \n" << e.what() << "\n\n" << std::endl;
		}
		catch (...) {
			std::cerr << "\nAn unknown exception occurred by LaserTrapping.\n" << std::endl;
		}
	}


	void Deposition::laserspot(cv::Mat& frame, double elapsedTime, cv::Mat& fullScreenImage) {
		int XaxisX2, YaxisY2;
		if ((pr.getLeft() - 30) < 0) {
			XaxisX2 = pr.getLeft();
		}
		else {
			XaxisX2 = pr.getLeft() - 30;
		}
		if ((pr.getTop() - 30) < 0) {
			YaxisY2 = pr.getTop();
		}
		else {
			YaxisY2 = pr.getTop() - 30;
		}
		cv::flip(frame, dframe, 1);//flip
		cv::rectangle(dframe, cv::Point(pr.getLeft(), pr.getTop()), cv::Point(pr.getLeft() + pr.getRadiusBox(), pr.getTop() + pr.gethofs()), pr.uBGR(0, 0, 255), 1);//red
		cv::rectangle(dframe, cv::Point(XaxisX2, YaxisY2), cv::Point(pr.getLeft() + pr.getRadiusBox() + 30, pr.getTop() + pr.gethofs() + 30), pr.uBGR(0, 255, 0), 1);
		//green
		cv::Rect roiRect(pr.getLeft() + 1, pr.getTop() + 1, pr.getRadiusBox() - 1, pr.gethofs() - 1);//small
		cv::Rect rRect(XaxisX2, YaxisY2, pr.getRadiusBox() + 63, pr.gethofs() + 63);//big

		grayColorRect = dframe(roiRect);//crop
		gRect = dframe(rRect);//crop

		setcurrentBrightness(grayColorRect);

		copyFrame(dframe, fullScreenImage, fwidth*0.1, 0, fwidth*0.40, fheight*0.5);
		//copyFrame(grayColorRect, fullScreenImage, fwidth / 3, 0, fwidth / 3, fheight / 2);//samall copy to second 
		copyFrame(gRect, fullScreenImage,  fwidth*0.50, 0, fwidth*0.50, fheight*0.5);//big copy to last 

		cv::Rect firstgraph(0, fheight * 0.55, fwidth * 0.75, fheight * 0.22);
		cv::Mat graapp = fullScreenImage(firstgraph);

		/*cv::Rect secondgraph(0, fheight * 0.70, fwidth * 0.75, fheight * 0.15);//x1,y1,w,h
		cv::Mat graappix = fullScreenImage(secondgraph);*/

		cv::Rect thirdgraph(0, fheight * 0.77, fwidth * 0.75, fheight * 0.22);
		cv::Mat heightgraph = fullScreenImage(thirdgraph);

		cv::Rect infoarea(fwidth * 0.75, fheight * 0.55, fwidth * 0.25, fheight * 0.45);
		cv::Mat information = fullScreenImage(infoarea);
		information = cv::Mat::ones(information.size(), information.type()) * 100;

		allgraph(graapp, contrastData, 560, "BD");
		//allgraph(graappix, sdValues, 74, "SD");
		allgraph(heightgraph, pztValues, pr.maxVolt(), "PZT");

		int barHeight = static_cast<int>((getcurrentBrightness()) * 100);
		int hightofbrightness = 100;
		Deposition::drawRectangle(fullScreenImage, fullScreenImage.cols * 0.99, fullScreenImage.rows * 0.5 - barHeight, fullScreenImage.cols, fullScreenImage.rows * 0.5, pr.uBGR(0, 0, 255), -1);

		int barHe = static_cast<double>((voltage) * 100);
		int highestvalueofvoltage = 100;
		Deposition::drawRectangle(fullScreenImage, 0, pr.maxVolt() * 100 - (barHe), 5, pr.maxVolt() * 100, pr.uBGR(0, 255, 0), -1);
		
		Deposition::drawRectangle(fullScreenImage, padding, padding, (fwidth * 0.10) - (padding), (fheight * 0.10) - (padding), pr.uBGR(0, 255, 0), -1);
		Deposition::drawRectangle(fullScreenImage, padding, (fheight * 0.10) + padding, (fwidth * 0.10) - (padding), (fheight * 0.20) - (padding), pr.uBGR(255, 0, 0), -1);
		Deposition::drawRectangle(fullScreenImage, padding, (fheight * 0.20) + padding, (fwidth * 0.10) - (padding), (fheight * 0.30) - (padding), pr.uBGR(0, 0, 255), -1);

		Deposition::drawRectangle(fullScreenImage, padding, padding, (fwidth * 0.10) - (padding), (fheight * 0.10) - (padding), pr.uBGR(0, 0, 255), 1, "Stsrt");
		Deposition::drawRectangle(fullScreenImage, padding, (fheight * 0.10) + padding, (fwidth * 0.10) - (padding), (fheight * 0.20) - (padding), pr.uBGR(0, 255, 0), 1, "Pause");
		Deposition::drawRectangle(fullScreenImage, padding, (fheight * 0.20) + padding, (fwidth * 0.10) - (padding), (fheight * 0.30) - (padding), pr.uBGR(255,0,0), 1, "Stop");

		int y = 30;
		drawText(information, double2string(elapsedTime, "T: ") + double2string(etime, "   THmax: "), 0, y, 0.5, pr.uBGR(0, 0, 255), 1);
		y += 30;
		drawText(information, double2string(pr.maxVolt() * 6, "Expected Height: ") + double2string(cHT, "  Real Height: "), 0, y, 0.5, pr.uBGR(0, 0, 255), 1);
		y += 30;
		drawText(information, double2string(pr.getUpperTh(), "Upper th point: ") + double2string(pr.getLowerTh(), "  lower th point: "), 0, y, 0.5, pr.uBGR(0, 0, 255), 1);
		y += 30;
		drawText(information, double2string(getcurrentBrightness(), "Real Brightness: ") + double2string(getEV(), "   EV: "), 0, y, 0.5, pr.uBGR(0, 0, 255), 1);
		y += 30;
		drawText(information, double2string(pr.getDurationTime(), "Expected Time: "), 0, y, 0.5, pr.uBGR(0, 0, 255), 1);
		y += 30;
		drawText(information, double2string(pr.getRadiusBox(), "Spot size: "), 0, y, 0.5, pr.uBGR(0, 0, 255), 1);
		y += 30;
		drawText(information, double2string(pr.maxVolt() * 6 / (numSteps() + timedelay), "V(micro-m/s): "), 0, y, 0.5, pr.uBGR(0, 0, 255), 1);
		y += 30;
		drawText(information, "file:" + exportfile, 0, y, 0.5, pr.uBGR(0, 0, 255), 1);
		y += 30;
		if (getEV() == 0.0) {
			drawRectangle(information, 0, y, 25, y + 20, pr.uBGR(0, 0, 255), -1);
		}
		else {
			drawRectangle(information, 0, y, 25, y + 20, pr.uBGR(0, 255, 0), -1);
		}
	}
	void Deposition::copyFrame(cv::Mat& frame, cv::Mat& screenImage, int x, int y, int x2, int y2) {
		// Validate that x, y, x2, and y2 are non-negative
		if (x < 0 || y < 0 || x2 < 0 || y2 < 0) {
			// Handle the error, e.g., print an error message or return early
			std::cerr << "Error: Invalid coordinates (x, y, x2, y2 must be non-negative)." << std::endl;
			return;
		}
		// Resize the frame to the specified dimensions (x2, y2)
		cv::resize(frame, frame, cv::Size(x2, y2));
		// Copy the resized frame into the screenImage at the specified position (x, y)
		frame.copyTo(screenImage(cv::Rect(x, y, frame.cols, frame.rows)));
	}

	void Deposition::allgraph(cv::Mat& frame, std::vector<double>& graphValues, double upperLimit, const std::string& yxix) {
		const int startPointX = 30;
		if (graphValues.empty()) {
			return;
		}
		int height = frame.rows;
		int width = frame.cols;
		if (graphValues.size() >= static_cast<size_t>(width - startPointX)) {
			// Erase elements from the beginning to keep the size within the limit
			graphValues.erase(graphValues.begin(), graphValues.begin() + (graphValues.size() - (width - startPointX)));
		}
		cv::Point startPoint(startPointX, height * 0.5);
		frame = cv::Scalar(255, 255, 255);

		for (int i = 0; i < graphValues.size(); ++i) {
			double y = (graphValues[i] / upperLimit) * (height * 0.8) + 10;
			cv::Point endPoint(i + startPointX, height - static_cast<int>(y));
			line(frame, startPoint, endPoint, cv::Scalar(0, 0, 0), 1);
			startPoint = endPoint;
		}
		drawYAxisValues(frame, pr.uBGR(0, 0, 0), upperLimit, yxix);
		drawXAxis(frame, pr.uBGR(0, 0, 0));
	}

	void Deposition::drawText(cv::Mat& frame, const std::string& text, int x, int y, double fontSize, const cv::Scalar& color, int thickness) {
		cv::putText(frame, text, cv::Point(x, y), cv::FONT_HERSHEY_SIMPLEX, fontSize, color, thickness);
	}

	void Deposition::drawRectangle(cv::Mat& frame, int x1, int y1, int x2, int y2, const cv::Scalar& color, int thickness , const std::string& text ="") {
		cv::rectangle(frame, cv::Point(x1, y1), cv::Point(x2, y2), color, thickness);
		cv::putText(frame, text, cv::Point(x1+10, (y1+y2)*0.5), cv::FONT_HERSHEY_SIMPLEX, 1, pr.uBGR(255, 255, 255), 1);
	}

	void Deposition::drawYAxisValues(cv::Mat& graphArea, const cv::Scalar& color, const double& text, const std::string& yaxis) {
		std::string tr1 = double2string(text, " ");
		std::string tr3 = double2string(text - text, " ");
		drawText(graphArea, tr1, -1, graphArea.rows * 0.15, 0.5, pr.uBGR(0, 0, 255), 1);
		drawText(graphArea, yaxis, -1, graphArea.rows * 0.55, 0.5, pr.uBGR(0, 255, 0), 1);
		drawText(graphArea, tr3, -1, graphArea.rows * 0.95, 0.5, pr.uBGR(0, 0, 255), 1);
	}

	void Deposition::drawXAxis(cv::Mat& graphArea, const cv::Scalar& color) {
		double thline = ((80 * (1 - pr.getUpperTh())) + 15) / 100;
		DrawDashedLine(graphArea, cv::Point(30, graphArea.rows * 0.15), cv::Point(graphArea.cols, graphArea.rows * 0.15), pr.uBGR(0, 0, 255), 1, "dotted", 10);
		line(graphArea, cv::Point(30, graphArea.rows * 0.15), cv::Point(30, graphArea.rows * 0.95), color, 1);
		cv::line(graphArea, cv::Point(30, graphArea.rows * 0.95), cv::Point(graphArea.cols, graphArea.rows * 0.95), color, 1, cv::LINE_8);
	}

	void Deposition::writeContrastToCSV(
		const std::string& filename, 
		const std::vector<double>& contrastData, 
		const std::vector<double>& data3, 
		/*const std::vector<double>& data4,*/
		const std::string& yaxis, 
		const std::string& name3) 
	{
		std::ofstream outFile(filename);
		if (!outFile.is_open()) {
			std::cerr << "Error opening file for writing." << std::endl;
			return;
		}
		outFile <<"SN,"+ yaxis + "," + name3 << std::endl;
		size_t maxSize = max(contrastData.size(), data3.size());

		size_t outputIndex = 1;
		for (size_t i = 0; i < maxSize; ++i) {
			if (data3[i] != 0.0) {
				outFile << outputIndex << ",";
				if (i < contrastData.size()) {
					outFile << contrastData[i];
				}
				outFile << ",";
				if (i < data3.size()) {
					outFile << data3[i];
				}
				outFile << std::endl;
				++outputIndex;
			}
		}
		outFile.close();
	}

	void Deposition::wToCSV(const std::string& filename, const std::string& name) {
		std::ofstream outFile(filename);
		if (!outFile.is_open()) {
			std::cerr << "Error opening file for writing." << std::endl;
			return;
		}
		outFile << name << std::endl;
		outFile.close();
	}

	void Deposition::DrawDashedLine(cv::Mat& img, cv::Point pt1, cv::Point pt2, cv::Scalar color, int thickness, std::string style, int gap) {
		float dx = pt1.x - pt2.x;
		float dy = pt1.y - pt2.y;
		float dist = std::hypot(dx, dy);
		std::vector<cv::Point> pts;
		for (int i = 0; i < dist; i += gap) {
			float r = static_cast<float>(i / dist);
			int x = static_cast<int>((pt1.x * (1.0 - r) + pt2.x * r) + .5);
			int y = static_cast<int>((pt1.y * (1.0 - r) + pt2.y * r) + .5);
			pts.emplace_back(x, y);
		}
		int pts_size = pts.size();
		if (style == "dotted") {
			for (int i = 0; i < pts_size; ++i) {
				cv::circle(img, pts[i], thickness, color, -1);
			}
		}
		else {
			cv::Point s = pts[0];
			cv::Point e = pts[0];

			for (int i = 0; i < pts_size; ++i) {
				s = e;
				e = pts[i];
				if (i % 2 == 1) {
					cv::line(img, s, e, color, thickness);
				}
			}
		}
	}

	double Deposition::stdev(std::vector<double> pixData) {
		int size = pixData.size();
		double bright = 0, sum = 0;
		double vari = 0;
		int countLastFive = 0;
		double variance = 0.0;
		double mean = 0;
		int expectedsize = 25;
		if (pixData.empty()) {
			return 0.0;
		}
		for (int i = size - expectedsize; i < size; ++i) {
			if (i >= 0) {
				sum += pixData[i];
				++countLastFive;
			}
		}
		mean = (countLastFive > 0) ? (sum / countLastFive) : 0.0;
		for (int i = size - expectedsize; i < size; ++i) {
			if (i >= 0) {
				variance += std::pow(pixData[i] - mean, 2);
			}
		}
		variance /= (countLastFive);
		bright = std::sqrt(variance);
		return bright;
	}

	double epv = 0.0;
	void setEV(double ephv = 2.0) {
		epv = ephv;
	}
	double getEV() {
		return epv;
	}
};

