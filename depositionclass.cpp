#include "depositionclass.h"
#include <conio.h>
#include "preference.h"

Deposition::Deposition() : fwidth(1200),fheight(750),exportfile(), elapsedTime(), averagediff(0){
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
	bool isComplete = false;
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
	try {
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
			cv::setMouseCallback("Status of deposition", Deposition::onMouse, &fullScreenImage);
			char key = cv::waitKey(1);
			if (key == 'q' || key == ' ') {
				cam.release();
				cv::destroyAllWindows();
				break;
			}
		}
	}
	catch (const std::exception& e) {
		std::cerr << "\nAn exception occurred: \n" << e.what() << "\n\n" << std::endl;
	}
	catch (...) {
		std::cerr << "\nAn unknown exception occurred.\n" << std::endl;
	}
}

void Deposition::application() {
	try {
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
			if (!isComplete && voltage> -1) {
				if (feedback > BRIGHTNESS) {
					voltage += VOLTAGE / (numSteps + timedelay);
					electrophoretic = 2.0;
				}
			}
			if (!isComplete && voltage >= 0) {
				if (feedback > BRIGHTNESS) {
					voltage += VOLTAGE / (numSteps + timedelay);
					electrophoretic = 2.0;
				}
				if (feedback < BRIGHTNESS) {
					timedelay += 1;
					voltage -= VOLTAGE / (numSteps*0.25);
					electrophoretic = 2.0;
				}
				if (voltage >= VOLTAGE && !isComplete) {
					etime = elapsedTime;
					isComplete = true;
					voltage -= VOLTAGE / numSteps;
					electrophoretic = 0.0;
					writeContrastToCSV(commonPath + exportfile + ".csv", contrastData, grphValues, feed_deque, "No of frame", "Contrast", "PZT volt");
					std::string mulpani = 
						"Time,"+double2string(elapsedTime, " ") + "\n" +
						"THmax," + double2string(etime, "") + "\n" +
						"file," + exportfile + "\n" +
						"Height," + double2string(VOLTAGE * 6, "") + "\n" +
						"C_th," + double2string(BRIGHTNESS, "") + "\n" +
						"V(micro-m/s)," + double2string(VOLTAGE * 6 / (numSteps + timedelay), "");
					wToCSV(commonPath + exportfile + "d.csv", mulpani);					
				}
			}
			if (isComplete && voltage > 0) {
				voltage -= VOLTAGE / (numSteps * 0.25);
				electrophoretic = 0.0;
				if (voltage < 0) {
					voltage = 0;
					cv::imwrite(commonPath + exportfile + ".jpg", fullScreenImage);
					writeContrastToCSV(commonPath + exportfile + "uc.csv", contrastData, grphValues, feed_deque, "No of frame", "Contrast", "PZT volt");
					std::string mulpani =
						"Time," + double2string(elapsedTime, " ") + "\n" +
						"THmax," + double2string(etime, "") + "\n" +
						"file," + exportfile + "\n" +
						"Height," + double2string(VOLTAGE * 6, "") + "\n" +
						"C_th," + double2string(BRIGHTNESS, "") + "\n" +
						"V(micro-m/s)," + double2string(VOLTAGE * 6 / (numSteps + timedelay), "");
					wToCSV(commonPath + exportfile + "duc.csv", mulpani);

					cv::destroyWindow(exportfile);
					break;
				}
			}
			DAQmxWriteAnalogF64(task2, 1, true, 10.0, DAQmx_Val_GroupByChannel, &electrophoretic, nullptr, nullptr);
			DAQmxWriteAnalogF64(task1, 1, true, 10.0, DAQmx_Val_GroupByChannel, &voltage, nullptr, nullptr);
			grphValues.push_back(voltage);
			grphVa.push_back(voltage);
			pr.simpleCSVsave(LAST_VOLT_FILE, voltage);
			cv::imshow(exportfile, fullScreenImage);
			cv::moveWindow(exportfile, 0, 0);
			char key = cv::waitKey(1);
			if (key == 'q' || key == ' ') {
				isComplete = true;
				voltage -= VOLTAGE / (numSteps * 0.1);
				electrophoretic = 0.0;
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
		std::cerr << "\nAn exception occurred: \n" << e.what() <<"\n\n"<< std::endl;
	}
	catch (...) {
		std::cerr << "\nAn unknown exception occurred.\n" << std::endl;
	}
}
	
void Deposition::onMouse(int event, int x, int y, int flags, void* userdata) {
	cv::Mat& frame = *static_cast<cv::Mat*>(userdata);
	frame(cv::Rect(0, 0, 200, 20)).setTo(cv::Scalar(255, 255, 255));
	if (event == cv::EVENT_MOUSEMOVE) {
		std::string coordinates = "Coordinates: (" + std::to_string(x) + ", " + std::to_string(y) + ")";
		cv::putText(frame, coordinates, cv::Point(10, 15), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 1);
	}
	cv::imshow("Status of deposition", frame);
	cv::moveWindow("Status of deposition", 0, 0);
}

void Deposition::laserspot(cv::Mat& frame, double elapsedTime, cv::Mat& fullScreenImage) {
	int XaxisX2,YaxisY2;
	if ((XaxisX1 - 30) < 0) {
		XaxisX2 = XaxisX1;
	}
	else {
		XaxisX2 = XaxisX1 - 30;
	}
	if ((YaxisY1 - 30) < 0) {
		YaxisY2 = YaxisY1;
	}
	else {
		YaxisY2 = YaxisY1 - 30;
	}
	cv::flip(frame, dframe, 1);
	cv::rectangle(dframe, POINT1, POINT2, red, 1);
	cv::rectangle(dframe, cv::Point(XaxisX2, YaxisY2), cv::Point(XaxisX1 + radius + 30, YaxisY1 + radius + 30), green, 1);

	cv::Rect roiRect(XaxisX1 + 1, YaxisY1 + 1, radius - 1, radius - 1);
	cv::Rect rRect(XaxisX2, YaxisY2, radius + 63, radius + 63);

	grayColorRect = dframe(roiRect);
	gRect = dframe(rRect);
	BrightnessClass bri(grayColorRect);
	contrast = bri.avg();
	contrastData.push_back(contrast);

	cv::resize(grayColorRect, grayColorRect, cv::Size(fwidth / 3, fheight/2));
	cv::resize(gRect, gRect, cv::Size(fwidth / 3, fheight / 2));
	cv::resize(dframe, dframe, cv::Size(fwidth / 3, fheight / 2));

	cv::Rect mains(0, 0, dframe.cols, dframe.rows);
	dframe.copyTo(fullScreenImage(mains));

	cv::Rect grayRectROI(fwidth / 3, 0, grayColorRect.cols, grayColorRect.rows);
	grayColorRect.copyTo(fullScreenImage(grayRectROI));

	cv::Rect gRectROI(2*fwidth / 3, 0, gRect.cols, gRect.rows);
	gRect.copyTo(fullScreenImage(gRectROI));

	cv::Mat graapp = fullScreenImage(cv::Rect(0, fheight / 2, fwidth, fheight / 4));
	cv::Mat graappix = fullScreenImage(cv::Rect(0, (3 * fheight / 4), fwidth, fheight / 4));
	
	pixData.push_back(contrast);
	/*if (pixData.size() > (fwidth - 30)) {
		pixData.pop_front();
	}*/
	feedback = stdev(pixData);
	feed_deque.push_back(feedback);
	allgraph(graapp, feed_deque, bri.getUpperlimit());
	allgraph(graappix, grphVa, VOLTAGE);
	int barHeight = static_cast<int>((feedback)*frame.rows * 0.5);
	Deposition::drawRectangle(fullScreenImage, fullScreenImage.cols * 0.99, fullScreenImage.rows * 0.5 - barHeight, fullScreenImage.cols, fullScreenImage.rows * 0.5, red, -1);

	int barHe = static_cast<double>((voltage)*fullScreenImage.rows*0.1);
	Deposition::drawRectangle(fullScreenImage, 0, fullScreenImage.rows*0.5 - barHe, 5, fullScreenImage.rows * 0.5, green, -1);

	std::string timeStr = double2string(elapsedTime, "T: ") +
		double2string(etime, " /THmax: ") +" /file:" + exportfile +
		double2string(VOLTAGE * 6, " /Height: ") +
		double2string(BRIGHTNESS, " /C_th: ") +
		double2string(VOLTAGE * 6 / (numSteps + timedelay), " /V(micro-m/s): ");

	cv::Mat frameii = graapp(cv::Rect(0,0, fwidth,15));
	drawText(frameii, timeStr, 0, 0, 0.5, red, 1);
	
	if (electrophoretic == 0.0) {
		drawRectangle(fullScreenImage, 0, 5, 25, 20, red, -1);
	}
	else {
		drawRectangle(fullScreenImage, 0, 5, 25, 20, green, -1);
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

void Deposition::verticalIndicator(cv::Mat& frame, double brightness) {
	int barHeight = static_cast<int>((brightness) * frame.rows*0.5);
	Deposition::drawRectangle(frame, frame.cols * 0.99, frame.rows*0.5 - barHeight, frame.cols, frame.rows*0.5,red,-1);
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
	double thline = ((80*(1-BRIGHTNESS))+15)/100;
	DrawDashedLine(graphArea, cv::Point(30, graphArea.rows * 0.15), cv::Point(graphArea.cols, graphArea.rows * 0.15), red, 1, "dotted", 10);
	line(graphArea, cv::Point(30, graphArea.rows*0.15), cv::Point(30, graphArea.rows*0.95), color, 1);
	cv::line(graphArea, cv::Point(30, graphArea.rows*0.95), cv::Point(graphArea.cols, graphArea.rows*0.95), color, 1, cv::LINE_8);
	DrawDashedLine(graphArea, cv::Point(30, graphArea.rows * thline), cv::Point(graphArea.cols, graphArea.rows * thline), green, 1, "dotted", 10);
}

void Deposition::writeContrastToCSV(const std::string& filename, const std::vector<double>& contrastData, const std::vector<double>& data3, const std::deque<double>& data4, const std::string& xaxis, const std::string& yaxis, const std::string& name3) {
	std::ofstream outFile(filename);
	if (!outFile.is_open()) {
		std::cerr << "Error opening file for writing." << std::endl;
		return;
	}
	outFile << xaxis + "," + yaxis + "," + name3 + "," + "SD" + "," + "min" << std::endl;
	size_t maxSize = max(contrastData.size(), data3.size());

	double min_value = DBL_MAX;
	for (const double& value : data4) {
		if (value != 0.0 && value < min_value) {
			min_value = value;
		}
	}

	for (size_t i = 0; i < maxSize; ++i) {
		outFile << i + 1 << ",";
		if (i < contrastData.size()) {
			outFile << contrastData[i];
		}
		outFile << ",";
		if (i < data3.size()) {
			outFile << data3[i];
		}
		outFile << ",";
		if (i < data4.size()) {
			outFile << data4[i];
		}
		outFile << ",";
		if (i < data4.size()) {
			outFile << min_value;
		}
		outFile << std::endl;
	}
	outFile.close();
}

void Deposition::wToCSV(const std::string& filename, const std::string& name) {
	std::ofstream outFile(filename);
	if (!outFile.is_open()) {
		std::cerr << "Error opening file for writing." << std::endl;
		return;
	}
	outFile <<name<< std::endl;
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

double Deposition::stdev(std::deque<double> pixData) {
	int size = pixData.size();
	double bright = 0, sum = 0;
	double vari = 0;
	int countLastFive = 0;
	double variance = 0.0;
	double mean = 0;
	int expectedsize = 30;

	//cout << "\nsize - " << size << endl;
	if (pixData.empty()) {
		return 0.0;
	}
	for (int i = size - expectedsize; i < size; ++i) {
		if (i >= 0) {
			//cout << "pixData - " << i << " : " << pixData[i] << endl;
			sum += pixData[i];
			++countLastFive;
		}
	}
	mean = (countLastFive > 0) ? (sum / countLastFive) : 0.0;
	//cout << "mean - " << mean << endl;
	for (int i = size - expectedsize; i < size; ++i) {
		if (i >= 0) {
			//cout << "(" << pixData[i] << "-" << mean << ") : " << pixData[i] - mean << endl;
			variance += std::pow(pixData[i] - mean, 2);
		}
	}
	variance /= (countLastFive);
	//cout << "variance - " << variance << endl;
	bright = std::sqrt(variance);
	//cout << "bright - " << bright << endl;
	return bright;
}
