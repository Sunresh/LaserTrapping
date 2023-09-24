#include "depositionclass.h"
#include <conio.h>
#include "preference.h"

Deposition::Deposition() : fwidth(1200),fheight(750),exportfile(), elapsedTime() {
	fwidth = GetSystemMetrics(SM_CXSCREEN)-10;
	fheight = GetSystemMetrics(SM_CYSCREEN)-90;
	double v1 = 0.0;
	DAQmxCreateTask("", &task1);
	DAQmxCreateTask("", &task2);
	DAQmxCreateAOVoltageChan(task1, dev1, "ao_channel", 0.0, 5.0, DAQmx_Val_Volts, nullptr);
	DAQmxCreateAOVoltageChan(task2, dev0, "ao_channel", 0.0, 5.0, DAQmx_Val_Volts, nullptr);
	DAQmxCfgSampClkTiming(task1, "", 10.0, DAQmx_Val_Rising, DAQmx_Val_ContSamps, 1);
	DAQmxCfgSampClkTiming(task2, "", 10.0, DAQmx_Val_Rising, DAQmx_Val_ContSamps, 1);
	DAQmxWriteAnalogF64(task2, 1, true, 10.0, DAQmx_Val_GroupByChannel, &v1, nullptr, nullptr);
	DAQmxWriteAnalogF64(task1, 1, true, 10.0, DAQmx_Val_GroupByChannel, &v1, nullptr, nullptr);
	DAQmxClearTask(task1);
	DAQmxClearTask(task2);
	cam.open(CAMERA);

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
}

void Deposition::setfwidth(int windowwidth) {
	if (fwidth > 0)
		fwidth = windowwidth;
	else
		throw std::invalid_argument("Height must be greater than 0.");
}
int Deposition::getfwidth() const {
	return fwidth;
}
void Deposition::setfheight(int windowHeight) {
	if (fheight > 0)
		fheight = windowHeight;
	else
		throw std::invalid_argument("Height must be greater than 0.");
}
int Deposition::getfheight() const {
	return fheight;
}
void Deposition::setOutputFileName(std::string filename = commonPath) {
	auto now = std::chrono::system_clock::now();
	auto time_t_now = std::chrono::system_clock::to_time_t(now);
	struct tm timeinfo;
	localtime_s(&timeinfo, &time_t_now);

	std::ostringstream oss;
	oss << std::put_time(&timeinfo, "%Y%m%d_%H%M_%S");
//	exportfile = filename + oss.str();
	exportfile = oss.str();
}

std::string Deposition::getOutputFileName() const {
	return exportfile;
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
	cout << "End"<<endl;
}
void Deposition::camera() {
	if (!cam.isOpened()) {
		return;
	}
	auto startTime = std::chrono::high_resolution_clock::now();
	cv::Mat fullScreenImage(fheight, fwidth, CV_8UC3, white);
	while (true) {
		cam >> frame;
		if (frame.empty()) {
			break;
		}
		getelapsedTime(startTime);
		laserspot(frame, elapsedTime, fullScreenImage);
		cv::imshow("Status of deposition", fullScreenImage);
		cv::moveWindow("Status of deposition", 0, 0);
		char key = cv::waitKey(1);
		if (key == 'q' || key == ' ') {
			cam.release();
			cv::destroyAllWindows();
			break;
		}
	}
}

void Deposition::application() {
	if (!cam.isOpened()) {
		return;
	}
	DAQmxCreateTask("", &task1);
	DAQmxCreateTask("", &task2);
	DAQmxCreateAOVoltageChan(task1, dev1, "ao_channel", 0.0, 5.0, DAQmx_Val_Volts, nullptr);
	DAQmxCreateAOVoltageChan(task2, dev0, "ao_channel", 0.0, 5.0, DAQmx_Val_Volts, nullptr);
	DAQmxCfgSampClkTiming(task1, "", 10.0, DAQmx_Val_Rising, DAQmx_Val_ContSamps, 1);
	DAQmxCfgSampClkTiming(task2, "", 10.0, DAQmx_Val_Rising, DAQmx_Val_ContSamps, 1);

	auto startTime = std::chrono::high_resolution_clock::now();
	cv::Mat fullScreenImage(fheight, fwidth, CV_8UC3, white);
	setOutputFileName();
	while (true) {
		cam >> dframe;
		if (dframe.empty()) {
			break;
		}
		getelapsedTime(startTime);
		laserspot(dframe, elapsedTime, fullScreenImage);

		if (isIncrease && 0 <= voltage) {
			if (contrast > BRIGHTNESS) {
				voltage += VOLTAGE / (numSteps + timedelay);
				electrophoretic = 2.0;
				pr.simpleCSVsave(LAST_VOLT_FILE, voltage);
			}
			if (contrast < BRIGHTNESS) {
				timedelay += 1;
				voltage -= VOLTAGE / numSteps;
				electrophoretic = 0.0;
				pr.simpleCSVsave(LAST_VOLT_FILE, voltage);
			}
			if (voltage >= VOLTAGE && !updated) {
				etime = elapsedTime;
				updated = true;
				isIncrease = false;
				voltage -= VOLTAGE / numSteps;
				electrophoretic = 0.0;
				pr.simpleCSVsave(LAST_VOLT_FILE, voltage);
			}
		}
		if (!isIncrease && voltage > 0) {
			voltage -= VOLTAGE / (numSteps * 0.25);
			electrophoretic = 0.0;
			if (voltage < 0) {
				voltage = 0;
				cv::imwrite(commonPath+exportfile+".jpg", fullScreenImage);
				writeContrastToCSV(commonPath+exportfile+"cntr.csv", contrastData, "Number of frame", "Contrast");
				writeContrastToCSV(commonPath+exportfile+"volt.csv", grphValues, "Number of frame", "PZT Voltage");
				cv::destroyWindow(exportfile);
				pr.simpleCSVsave(LAST_VOLT_FILE, voltage);
				//return app(); 
				break;
			}
		}
		DAQmxWriteAnalogF64(task2, 1, true, 10.0, DAQmx_Val_GroupByChannel, &electrophoretic, nullptr, nullptr);
		DAQmxWriteAnalogF64(task1, 1, true, 10.0, DAQmx_Val_GroupByChannel, &voltage, nullptr, nullptr);
		grphValues.push_back(voltage);
		grphVa.push_back(voltage);

		cv::imshow(exportfile, fullScreenImage);
		cv::moveWindow(exportfile, 0, 0);
		char key = cv::waitKey(1);
		if (key == 'q' || key == ' ') {
			cam.release();
			cv::destroyAllWindows();
			break;
		}
	}
	DAQmxClearTask(task1);
	DAQmxClearTask(task2);
}

void Deposition::laserspot(cv::Mat& frame, double elapsedTime, cv::Mat& fullScreenImage) {
	cv::flip(frame, dframe, 1);
	cv::rectangle(dframe, POINT1, POINT2, red, 1);
	cv::rectangle(dframe, cv::Point(XaxisX1 - 30, YaxisY1 - 30), cv::Point(XaxisX1 + radius + 30, YaxisY1 + radius + 30), green, 1);

	cv::Rect roiRect(XaxisX1 + 1, YaxisY1 + 1, radius - 1, radius - 1);
	cv::Rect rRect(XaxisX1 - 31, YaxisY1 - 31, radius + 63, radius + 63);

	grayColorRect = dframe(roiRect);
	gRect = dframe(rRect);

	contrast = calculateContrast(grayColorRect);
	contrastData.push_back(contrast);

	std::string timeStr = double2string(elapsedTime, "T: ") + 
		double2string(etime, " /THmax: ") + 
		" /file:" + exportfile + 
		double2string(VOLTAGE*6, " /Height: ") + 
		double2string(electrophoretic, " /EPV: ")+ 
		double2string(BRIGHTNESS, " /C_th: ")+
		double2string(voltage * 6, " /H: ")+ 
		double2string(contrast, " /C: ")+ 
		double2string(VOLTAGE * 6 / (numSteps + timedelay), " /V(micm/s): ");
	/*if (grphVa.size() > (fwidth - 30)) {
		grphVa.pop_front();
	}*/
	cv::resize(grayColorRect, grayColorRect, cv::Size(fwidth / 3, fheight/2));
	cv::resize(gRect, gRect, cv::Size(fwidth / 3, fheight / 2));
	cv::resize(dframe, dframe, cv::Size(fwidth / 3, fheight / 2));

	cv::Rect mains(0, 0, dframe.cols, dframe.rows);
	dframe.copyTo(fullScreenImage(mains));

	cv::Rect grayRectROI(fwidth / 3, 0, grayColorRect.cols, grayColorRect.rows);
	grayColorRect.copyTo(fullScreenImage(grayRectROI));

	// Display gRect in fullScreenImage
	cv::Rect gRectROI(2*fwidth / 3, 0, gRect.cols, gRect.rows);
	gRect.copyTo(fullScreenImage(gRectROI));

	cv::Mat graapp = fullScreenImage(cv::Rect(0, fheight / 2, fwidth, fheight / 4));
	cv::Mat graappix = fullScreenImage(cv::Rect(0, (3 * fheight / 4), fwidth, fheight / 4));

	pixData.push_back(contrast);
	/*if (pixData.size() > (fwidth - 30)) {
		pixData.pop_front();
	}*/
	allgraph(graapp, pixData, 1);
	allgraph(graappix, grphVa, VOLTAGE);

	int y = 30;
	drawText(fullScreenImage, timeStr, 5, y, 0.5, red, 1);
	y += 30;
	if (electrophoretic == 0.0) {
		drawRectangle(fullScreenImage, 0, y, 25, y+15, red, -1);
	}
	else {
		drawRectangle(fullScreenImage, 0, y, 25, y+15, green, -1);
	}
}

void Deposition::allgraph(cv::Mat& frame, const std::deque<double>& graphValues,double upperLimit) {
	const int startPointX = 30;
	const int lineThickness = 1;
	if (graphValues.empty()) {
		return;
	}
	int height = frame.rows;
	int width = frame.cols;
	cv::Point startPoint(startPointX, frame.rows * 0.5);
	for (int i = 0; i < graphValues.size(); ++i) {
		double y = (graphValues[i] / upperLimit) * (height*0.8)+ 10;
		cv::Point endPoint(i + startPointX, height - static_cast<int>(y));
		line(frame, startPoint, endPoint, cv::Scalar(0, 0, 0), lineThickness);
		startPoint = endPoint;
	}
	drawYAxisValues(frame, black, upperLimit);
	drawXAxis(frame, black);
}

void Deposition::drawText(cv::Mat& frame, const std::string& text, int x, int y, double fontSize, const cv::Scalar& color , int thickness) {
	cv::putText(frame, text, cv::Point(x, y), cv::FONT_HERSHEY_SIMPLEX, fontSize, color, thickness);
}

void Deposition::drawRectangle(cv::Mat& frame, int x1, int y1, int x2, int y2, const cv::Scalar& color, int thickness) {
	cv::rectangle(frame, cv::Point(x1, y1), cv::Point(x2, y2), color, thickness);
}

void Deposition::drawYAxisValues(cv::Mat& graphArea,const cv::Scalar& color, const double& text) {
	std::string tr1 = double2string(text, " ");
	std::string tr2 = double2string(text/2, " ");
	std::string tr3 = double2string(text- text, " ");

	drawText(graphArea, tr1, -1, graphArea.rows*0.15, 0.5, red, 1);
	drawText(graphArea, tr2, -1, graphArea.rows*0.55, 0.5, red, 1);
	drawText(graphArea, tr3, -1, graphArea.rows*0.95, 0.5, red, 1);
}

void Deposition::drawXAxis(cv::Mat& graphArea, const cv::Scalar& color) {
	DrawDashedLine(graphArea, cv::Point(30, graphArea.rows*0.15), cv::Point(graphArea.cols, graphArea.rows*0.15), red, 1, "dotted", 10);
	line(graphArea, cv::Point(30, graphArea.rows*0.15), cv::Point(30, graphArea.rows*0.95), color, 1);//verticle
	cv::line(graphArea, cv::Point(30, graphArea.rows*0.95), cv::Point(graphArea.cols, graphArea.rows*0.95), color, 1, cv::LINE_8);//horizontal
}

double Deposition::calculateContrast(const cv::Mat& frame) {
	cv::Mat grayFrame;
	cv::cvtColor(frame, grayFrame, cv::COLOR_BGR2GRAY);
	int width = grayFrame.cols;
	int height = grayFrame.rows;
	double sumPij = 0.0;
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			sumPij += grayFrame.at<uchar>(y, x);
		}
	}
	double contrast = sumPij / (255 * width * height);
	return contrast;
}

void Deposition::writeContrastToCSV(const std::string& filename, const std::vector<double>& contrastData, const std::string& xaxis, const std::string& yaxis) {
	std::ofstream outFile(filename);
	if (!outFile.is_open()) {
		std::cerr << "Error opening file for writing." << std::endl;
		return;
	}
	outFile << xaxis + ","+ yaxis << std::endl;
	for (int i = 0; i < contrastData.size(); ++i) {
		outFile << i + 1 << "," << contrastData[i] << std::endl;
	}
	outFile.close();
}
void Deposition::DrawDashedLine(cv::Mat& img, cv::Point pt1, cv::Point pt2,cv::Scalar color, int thickness, std::string style,int gap) {
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