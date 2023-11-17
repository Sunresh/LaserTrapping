#include "depositionclass.h"
#include <conio.h>
#include "preference.h"

Deposition::Deposition() : 
	fwidth(1200),fheight(750),exportfile(), elapsedTime(),
	averagediff(0),cBR(0),cHT(0),isCameraOnly(false)
{
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
void Deposition::setcurrentBrightness(cv::Mat& frame) {
	BrightnessClass bri(frame);
	double contrast = bri.avg();
	cBR = contrast;
}
double Deposition::getcurrentBrightness() {
	return cBR;
}
void Deposition::setcurrentHeight(double voltage) {
	double height = 6*voltage;
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
	sis << stri << std::fixed << std::setprecision(1) << value;
	std::string thrr = sis.str();
	return thrr;
}

Deposition::~Deposition() {
	cam.release();
	cv::destroyAllWindows();
	cout << "End"<<endl;
}
void Deposition::camera() {
	isCameraOnly = true;
	application();
}

void Deposition::application() {
	try {
		if (!cam.isOpened()) {
			return;
		}
		if (!isCameraOnly) {
			DAQmxCreateTask("", &task1);
			DAQmxCreateTask("", &task2);
			DAQmxCreateAOVoltageChan(task1, dev1, "ao_channel", 0.0, 5.0, DAQmx_Val_Volts, nullptr);
			DAQmxCreateAOVoltageChan(task2, dev0, "ao_channel", 0.0, 5.0, DAQmx_Val_Volts, nullptr);
			DAQmxCfgSampClkTiming(task1, "", 10.0, DAQmx_Val_Rising, DAQmx_Val_ContSamps, 1);
			DAQmxCfgSampClkTiming(task2, "", 10.0, DAQmx_Val_Rising, DAQmx_Val_ContSamps, 1);
		}
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
			SchmittTrigger schmittTrigger(BRIGHTNESS, LOWER_SD_POINT); // Set upper and lower thresholds
			bool output = schmittTrigger.processInput(feedback);
			if (!isCameraOnly) {
				if (!isComplete) {
					if (voltage < 0) {
						voltage = 0.0;
						timedelay = 0.0;
					}
					if (output && isWithoutredeposition && !isRedeposition) {
						voltage += pr.maxVolt() / (numSteps);
						electrophoretic = 2.0;
					}
					if (output && isRedeposition) {
						voltage += pr.maxVolt() / (numSteps + timedelay);
						electrophoretic = 2.0;
					}
					if (!output) {
						timedelay += 1;
						voltage -= pr.maxVolt() / (numSteps * 0.25);
						electrophoretic = 2.0;
						isRedeposition = true;
						isWithoutredeposition = false;
					}
					if (voltage >= pr.maxVolt() && !isComplete) {
						etime = elapsedTime;
						isComplete = true;
						voltage -= pr.maxVolt() / numSteps;
						electrophoretic = 0.0;
						writeContrastToCSV(commonPath + exportfile + "top.csv", contrastData, grphValues, feed_deque, "No of frame", "Contrast", "PZT volt");
					}
				}
				if (isComplete) {
					voltage -= pr.maxVolt() / (numSteps * 0.2);
					electrophoretic = 0.0;
					if (voltage < 0) {
						voltage = 0;
						cv::imwrite(commonPath + exportfile + ".jpg", fullScreenImage);
						writeContrastToCSV(commonPath + exportfile + "sd.csv", contrastData, grphValues, feed_deque, "No of frame", "Contrast", "PZT volt");
						std::string mulpani =
							"Time," + double2string(elapsedTime, " ") + "\n" +
							"THmax," + double2string(etime, "") + "\n" +
							"file," + exportfile + "\n" +
							"Height," + double2string(pr.maxVolt() * 6, "") + "\n" +
							"C_th," + double2string(BRIGHTNESS, "") + "\n" +
							"V(micro-m/s)," + double2string(pr.maxVolt() * 6 / (numSteps + timedelay), "");
						wToCSV(commonPath + exportfile + "debug.csv", mulpani);

						cv::destroyWindow(exportfile);
						break;
					}
				}
				DAQmxWriteAnalogF64(task2, 1, true, 10.0, DAQmx_Val_GroupByChannel, &electrophoretic, nullptr, nullptr);
				DAQmxWriteAnalogF64(task1, 1, true, 10.0, DAQmx_Val_GroupByChannel, &voltage, nullptr, nullptr);

				grphValues.push_back(voltage);
				grphVa.push_back(voltage);
				pr.simpleCSVsave(LAST_VOLT_FILE, voltage);
			}
			cv::imshow(exportfile, fullScreenImage);
			cv::moveWindow(exportfile, 0, 0);
			char key = cv::waitKey(1);
			if (key == 'q' || key == ' ') {
				isComplete = true;
				voltage -= pr.maxVolt() / (numSteps * 0.1);
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
	cv::flip(frame, dframe, 1);//flip
	cv::rectangle(dframe, POINT1, POINT2, red, 1);//red
	cv::rectangle(dframe, cv::Point(XaxisX2, YaxisY2), cv::Point(XaxisX1 + radius + 30, YaxisY1 + radius + 30), green, 1);
	//green
	cv::Rect roiRect(XaxisX1 + 1, YaxisY1 + 1, radius - 1, radius - 1);//small
	cv::Rect rRect(XaxisX2, YaxisY2, radius + 63, radius + 63);//big

	grayColorRect = dframe(roiRect);//crop
	gRect = dframe(rRect);//crop

	setcurrentBrightness(grayColorRect);
	contrastData.push_back(getcurrentBrightness());

	copyFrame(dframe, fullScreenImage, 0, 0, fwidth / 3, fheight / 2);//ogiginal camera copy to fullscreen 
	copyFrame(grayColorRect, fullScreenImage, fwidth / 3, 0, fwidth / 3, fheight / 2);//samall copy to second 
	copyFrame(gRect, fullScreenImage, 2 * fwidth / 3, 0, fwidth / 3, fheight / 2);//big copy to last 

	cv::Rect firstgraph(0, fheight*0.55, fwidth*0.75, fheight*0.15);
	cv::Mat graapp = fullScreenImage(firstgraph);

	cv::Rect secondgraph(0, fheight * 0.70, fwidth*0.75, fheight*0.15);//x1,y1,w,h
	cv::Mat graappix = fullScreenImage(secondgraph);

	cv::Rect thirdgraph(0, fheight * 0.85, fwidth*0.75, fheight*0.15);
	cv::Mat heightgraph = fullScreenImage(thirdgraph);

	cv::Rect infoarea(fwidth * 0.75, fheight * 0.55, fwidth * 0.25, fheight * 0.45);
	cv::Mat information = fullScreenImage(infoarea);
	information = cv::Mat::ones(information.size(), information.type())*100;


	pixData.push_back(getcurrentBrightness());
	
	feedback = stdev(pixData);
	feed_deque.push_back(feedback);
	allgraph(graapp, pixData, 1,"Brightness");
	allgraph(graappix,  feed_deque, 0.3,"SD");
	allgraph(heightgraph, grphVa, pr.maxVolt(),"PZT");

	int barHeight = static_cast<int>((feedback)*100);
	int hightofbrightness = 100;
	Deposition::drawRectangle(fullScreenImage, fullScreenImage.cols * 0.99, fullScreenImage.rows * 0.5 - barHeight, fullScreenImage.cols, fullScreenImage.rows * 0.5, red, -1);

	int barHe = static_cast<double>((voltage)*100);
	int highestvalueofvoltage = 100;
	Deposition::drawRectangle(fullScreenImage, 0, pr.maxVolt() *100 - (barHe), 5, pr.maxVolt() *100, green, -1);

	int y = 30;
	drawText(information, double2string(elapsedTime, "T: "), 0, y, 0.5, red, 1);
	y += 30;
	drawText(information, double2string(etime, "THmax: "), 0, y, 0.5, red, 1);
	y += 30;
	drawText(information, double2string(pr.maxVolt() * 6, "Height: "), 0, y, 0.5, red, 1);
	y += 30;
	drawText(information, double2string(cHT, "cHeight: "), 0, y, 0.5, red, 1);
	y += 30;
	drawText(information, double2string(BRIGHTNESS, "C_th: "), 0, y, 0.5, red, 1);
	y += 30;
	drawText(information, double2string(pr.maxVolt() * 6 / (numSteps + timedelay), "V(micro-m/s): "), 0, y, 0.5, red, 1);
	y += 30;
	drawText(information, "file:" + exportfile, 0, y, 0.5, red, 1);
	y += 30;
	if (electrophoretic == 0.0) {
		drawRectangle(information, 0, y, 25, y+20, red, -1);
	}
	else {
		drawRectangle(information, 0, y, 25, y+20, green, -1);
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

void Deposition::allgraph(cv::Mat& frame, std::deque<double>& graphValues,double upperLimit, const std::string& yxix) {
	const int startPointX = 30;
	if (graphValues.empty()) {
		return;
	}
	int height = frame.rows;
	int width = frame.cols;
	if (graphValues.size() >= width - startPointX) {
		graphValues.pop_front();
	}
	cv::Point startPoint(startPointX, height * 0.5);
	frame = cv::Scalar(255, 255, 255);

	for (int i = 0; i < graphValues.size(); ++i) {
		double y = (graphValues[i] / upperLimit) * (height*0.8)+ 10;
		cv::Point endPoint(i + startPointX, height - static_cast<int>(y));
		line(frame, startPoint, endPoint, cv::Scalar(0, 0, 0), 1);
		startPoint = endPoint;
	}
	//double newYValue = 50;
	//graphValues.push_back(newYValue);

	drawYAxisValues(frame, black, upperLimit, yxix);
	drawXAxis(frame, black);
}

void Deposition::drawText(cv::Mat& frame, const std::string& text, int x, int y, double fontSize, const cv::Scalar& color , int thickness) {
	cv::putText(frame, text, cv::Point(x, y), cv::FONT_HERSHEY_SIMPLEX, fontSize, color, thickness);
}

void Deposition::drawRectangle(cv::Mat& frame, int x1, int y1, int x2, int y2, const cv::Scalar& color, int thickness) {
	cv::rectangle(frame, cv::Point(x1, y1), cv::Point(x2, y2), color, thickness);
}

void Deposition::drawYAxisValues(cv::Mat& graphArea,const cv::Scalar& color, const double& text, const std::string& yaxis) {
	std::string tr1 = double2string(text, " ");
	std::string tr3 = double2string(text- text, " ");
	drawText(graphArea, tr1, -1, graphArea.rows*0.15, 0.5, red, 1);
	drawText(graphArea, yaxis, -1, graphArea.rows*0.55, 0.5, green, 1);
	drawText(graphArea, tr3, -1, graphArea.rows*0.95, 0.5, red, 1);
}

void Deposition::drawXAxis(cv::Mat& graphArea, const cv::Scalar& color) {
	double thline = ((80*(1-BRIGHTNESS))+15)/100;
	DrawDashedLine(graphArea, cv::Point(30, graphArea.rows * 0.15), cv::Point(graphArea.cols, graphArea.rows * 0.15), red, 1, "dotted", 10);
	line(graphArea, cv::Point(30, graphArea.rows*0.15), cv::Point(30, graphArea.rows*0.95), color, 1);
	cv::line(graphArea, cv::Point(30, graphArea.rows*0.95), cv::Point(graphArea.cols, graphArea.rows*0.95), color, 1, cv::LINE_8);
	//DrawDashedLine(graphArea, cv::Point(30, graphArea.rows * thline), cv::Point(graphArea.cols, graphArea.rows * thline), green, 1, "dotted", 10);
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
	int expectedsize = 25;
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
