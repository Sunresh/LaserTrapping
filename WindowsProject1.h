#pragma once
#include "resource.h"
#include "framework.h"
#include "WindowsProject1.h"
#include "opencv2/opencv.hpp"

class Camera {
private:
	int id = 0;
	cv::VideoCapture cap;
	int getId;
	bool stopCamera = true;
	std::deque<double> brightData;
	double brightness = 0;

public:
	Camera() : getId(id) {

	}
	void setStopCamera(bool stop) {
		stopCamera = stop;
	}
	bool getstopCamera() const {
		return stopCamera;
	}
	void start() {
		OutputDebugStringW(L"\naayyo\n");
	}
	void drawRectangle(cv::Mat& frame, int x1, int y1, int x2, int y2, const cv::Scalar& color, int thickness = 1) {
		cv::rectangle(frame, cv::Point(x1, y1), cv::Point(x2, y2), color, thickness);
	}
	double meanofBri(cv::Mat& iframe) {
		cv::Mat grayFrame;
		cv::cvtColor(iframe, grayFrame, cv::COLOR_BGR2GRAY);
		double width = grayFrame.cols;
		double height = grayFrame.rows;
		double variance = 0.0;
		double meanCV = cv::mean(grayFrame)[0];
		return meanCV;
	}
	const std::deque<double>& Camera::GetBrightData() const {
		return brightData;
	}
	void UpdateBrightData() {
		// Perform modifications or updates to the deque here
		// For example, to add a new point to the front:
		brightData.push_front(getBrightness());

		// Optionally, you might want to limit the size of brightData
		// For example, to maintain a certain maximum size:
		const size_t maxSize = 100; // Set your desired maximum size
		while (brightData.size() > maxSize) {
			brightData.pop_back();
		}
	}
	void setBrightness(cv::Mat tmpcalcFrame) {
		brightness = meanofBri(tmpcalcFrame);
	}
	const double getBrightness() const{
		return brightness;
	}
	void DisplayCameraFrame(HWND hWnd, HWND hWn, HWND calframe)
	{
		cap.open(0);
		if (!cap.isOpened()) {
			return;
		}
		HDC hdc = GetDC(hWnd); // Obtain device context for the window
		HDC hdc1 = GetDC(hWn); // Obtain device context for the window
		HDC hdcCalframe = GetDC(calframe);
		cv::Mat dframe, frame;
		while (!getstopCamera()) {
			cap >> dframe;
			if (!dframe.empty()) {
				cv::flip(dframe, frame, 1);//flip
				cv::resize(frame, frame, cv::Size(260, 215));
				// Show original frame in g_hFrame1
				cv::Mat tmpFrameOriginal;
				cv::cvtColor(frame, tmpFrameOriginal, cv::COLOR_BGR2BGRA);
				// Show cropped portion in zoomfram
				drawRectangle(tmpFrameOriginal, 50, 50, 150, 150, cv::Scalar(0, 0, 255), 1);
				cv::Mat croppedFrame = frame(cv::Rect(50, 50, 100, 100)); // Example crop - adjust as needed
				cv::Mat tmpFrameCropped;
				cv::cvtColor(croppedFrame, tmpFrameCropped, cv::COLOR_BGR2BGRA);
				cv::resize(tmpFrameCropped, tmpFrameCropped, cv::Size(260, 215));
				// Show calfram portion in zoomfram
				drawRectangle(tmpFrameCropped, 50, 50, 150, 150, cv::Scalar(0, 0, 255), 1);
				cv::Mat calcFrame = tmpFrameCropped(cv::Rect(50, 50, 100, 100)); // Example crop - adjust as needed
				cv::Mat tmpcalcFrame;
				cv::cvtColor(calcFrame, tmpcalcFrame, cv::COLOR_BGR2BGRA);
				cv::resize(tmpcalcFrame, tmpcalcFrame, cv::Size(260, 215));

				setBrightness(tmpcalcFrame);
				brightData.push_back(getBrightness());
				std::wstring meanString = std::to_wstring(getBrightness());
				OutputDebugStringW(meanString.c_str());

				// Display original frame in g_hFrame1
				HBITMAP hBitmapOriginal = CreateBitmap(tmpFrameOriginal.cols, tmpFrameOriginal.rows, 1, 32, tmpFrameOriginal.data);
				HDC memDCOriginal = CreateCompatibleDC(hdc);
				SelectObject(memDCOriginal, hBitmapOriginal);
				BitBlt(hdc, 0, 0, tmpFrameOriginal.cols, tmpFrameOriginal.rows, memDCOriginal, 0, 0, SRCCOPY);
				DeleteDC(memDCOriginal);
				DeleteObject(hBitmapOriginal);
				// Display cropped portion in zoomfram
				HBITMAP hBitmapCropped = CreateBitmap(tmpFrameCropped.cols, tmpFrameCropped.rows, 1, 32, tmpFrameCropped.data);
				HDC memDCCropped = CreateCompatibleDC(hdc1);
				SelectObject(memDCCropped, hBitmapCropped);
				BitBlt(hdc1, 0, 0, tmpFrameCropped.cols, tmpFrameCropped.rows, memDCCropped, 0, 0, SRCCOPY);
				DeleteDC(memDCCropped);
				DeleteObject(hBitmapCropped);
				// Display calframe portion in zoomfram
				HBITMAP hBitmapcalframe = CreateBitmap(tmpcalcFrame.cols, tmpcalcFrame.rows, 1, 32, tmpcalcFrame.data);
				HDC memDCCalcframe = CreateCompatibleDC(hdcCalframe);
				SelectObject(memDCCalcframe, hBitmapcalframe);
				BitBlt(hdcCalframe, 0, 0, tmpcalcFrame.cols, tmpcalcFrame.rows, memDCCalcframe, 0, 0, SRCCOPY);
				DeleteDC(memDCCalcframe);
				DeleteObject(hBitmapcalframe);
			}
			InvalidateRect(hWnd, NULL, TRUE);
			if (cv::waitKey(1) == 'q') {
				stopCamera = true;
				break;
			}
		}
		cap.release();
		ReleaseDC(hWnd, hdc);
		ReleaseDC(hWn, hdc1);
		ReleaseDC(calframe, hdcCalframe);
	}


};

#include <nidaqmx.h>
#include <iostream>
#include <string>
#include <vector>

class MyDaq {
private:
	TaskHandle analogTask;
	TaskHandle digitalTask;

public:
	MyDaq() : analogTask(nullptr), digitalTask(nullptr) {
		// Create tasks for analog and digital outputs
		DAQmxCreateTask("AnalogTask", &analogTask);
		DAQmxCreateTask("DigitalTask", &digitalTask);
	}

	~MyDaq() {
		// Clear tasks on destruction
		if (analogTask != nullptr) {
			DAQmxStopTask(analogTask);
			DAQmxClearTask(analogTask);
		}
		if (digitalTask != nullptr) {
			DAQmxStopTask(digitalTask);
			DAQmxClearTask(digitalTask);
		}
	}

	void addAnalogChannel(const char* channel) {
		DAQmxCreateAOVoltageChan(analogTask, channel, "", 0.0, 5.0, DAQmx_Val_Volts, nullptr);
	}

	void addDigitalChannel(const char* channel) {
		DAQmxCreateDOChan(digitalTask, channel, "", DAQmx_Val_ChanForAllLines);
	}

	void startTasks() {
		DAQmxStartTask(analogTask);
		DAQmxStartTask(digitalTask);
	}

	void analogOut(const char* channel, double voltage) {
		uInt32 writtenSamples;
		DAQmxWriteAnalogScalarF64(analogTask, true, 10.0, voltage, nullptr);
	}

	void digitalOut(const char* channel, bool value) {
		uInt32 data = value ? 1 : 0;
		DAQmxWriteDigitalScalarU32(digitalTask, true, 10.0, data, nullptr);
	}

	void handleError(int32 error) {
		if (DAQmxFailed(error)) {
			char errBuff[2048] = { '\0' };
			DAQmxGetExtendedErrorInfo(errBuff, 2048);

			std::cerr << "DAQmx Error: " << errBuff << std::endl;
		}
	}
};

class graphDraw {
private:
	int mal = 0;
public:
	void mayalau() {

	}
};
