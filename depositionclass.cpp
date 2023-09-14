#include "depositionclass.h"
#include <conio.h>
#include "preference.h"

Deposition::Deposition() {
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

	std::vector<UserPreferences> storedPreferences = pr.readFromCSV(filename);
	
	for (const UserPreferences& prefs : storedPreferences) {
		pztvolt = prefs.voltage;
		thContrast = prefs.threshold;
		durationInSeconds = prefs.time;
	}
	cam.open(0);
}

Deposition::~Deposition() {
	cam.release();
	cv::destroyAllWindows();
	cout << "End"<<endl;
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

	int numSteps = durationInSeconds * 100;
	double startVolt = 0.0;
	double endVolt = pztvolt;

	std::stringstream pztvoltage;
	pztvoltage << "Applied Voltage(PZT Stage): " << endVolt << "V";
	std::string pztStr = pztvoltage.str();
	std::stringstream thresholdp;
	thresholdp << "Threshold Contrast value: " << thContrast << " ";
	std::string thrr = thresholdp.str();

	auto now = std::chrono::system_clock::now();
	auto time_t_now = std::chrono::system_clock::to_time_t(now);
	struct tm timeinfo;
	localtime_s(&timeinfo, &time_t_now); // Platform-specific, for Windows
	char filename[80];
	char filena[80];
	char ima[80];
	char filee[80];


	strftime(filename, sizeof(filename), (commonPath + "%Y%m%d_%H%M%S_screen.jpg").c_str(), &timeinfo);
	strftime(filena, sizeof(filena), (commonPath + "%Y%m%d_%H%M%S_contrast.csv").c_str(), &timeinfo);
	strftime(ima, sizeof(ima), "%Y%m%d_%H%M%S", &timeinfo);
	strftime(filee, sizeof(filee), (commonPath + "%Y%m%d_%H%M%S_voltage.csv").c_str(), &timeinfo);

	auto startTime = std::chrono::high_resolution_clock::now();
	double etime = 0;
	bool updated = false;
	bool isIncrease = true;
	std::deque<double> lla;
	double voltage = startVolt;
	double electrophoretic = 0.0;
	cv::Mat dframe, grayColorRect;
	std::vector<double> contrastData;
	std::deque<double> pixData;
	std::vector<double> grphValues;
	std::deque<double> grphVa;
	int timedelay = 0;
	cv::Mat fullScreenImage(700, 1100, CV_8UC3, cv::Scalar(255, 255, 255)); // Red background
	int fwidth = fullScreenImage.cols;//dframe
	int fheight = fullScreenImage.rows;
	for (int i = 0; i <= numSteps * 2; i++) {
		cam >> dframe;
		if (dframe.empty()) {
			break;
		}
		cv::rectangle(dframe, pt1, pt2, red, 1);
		cv::Rect roiRect(x1+1, y1+1, roiw-2, roih-2); // (x, y, width, height)
		grayColorRect = dframe(roiRect).clone();
		double contrast = calculateContrast(grayColorRect);
		contrastData.push_back(contrast);
		
		std::stringstream con;
		con << "Running Constrast: " << contrast<<" per frame";
		std::string constr = con.str();

		string status;
		
		if (isIncrease && 0 <= voltage) {
			if (i == 0) {
				voltage += endVolt / numSteps;
				status = "Starting..";
				cout << status << endl;
				electrophoretic = 2.0;
			}
			if (contrast>thContrast) {
				status = "Deposition is going on..";
				voltage += endVolt / (numSteps + timedelay);
				electrophoretic = 2.0;
			}
			if ( contrast < thContrast ) {
				status = "Stage up..";
				numSteps += 3;
				timedelay += 1;
				voltage -= endVolt / numSteps;
				electrophoretic = 0.0;
			}
			if (voltage >= endVolt && !updated) {
				status = "Target Point.";
				cout << status << endl;
				auto currenTime = std::chrono::high_resolution_clock::now();
				std::chrono::duration<double> ella = currenTime - startTime;
				etime = ella.count();
				updated = true;
				isIncrease = false;
				voltage -= endVolt / numSteps;
				electrophoretic = 0.0;
			}
		}
		if (!isIncrease && voltage > 0) {
			status = "Deposition finished..";
			voltage -= endVolt / (numSteps * 0.25);
			electrophoretic = 0.0;
			if (voltage < 0) {
				voltage = 0;
				status = "Deposition completed..";
				std::string fullFilename = std::string(filename);
				std::string fullFilena = std::string(filena);
				std::string fullFile = std::string(filee);
				cv::imwrite(fullFilename, fullScreenImage);
				writeContrastToCSV(fullFilena, contrastData, "Number of frame", "Contrast");
				writeContrastToCSV(fullFile, grphValues, "Number of frame", "PZT Voltage");
				cv::destroyWindow("Status of deposition");
				//return app(); 
				break;
			}
		}
		DAQmxWriteAnalogF64(task2, 1, true, 10.0, DAQmx_Val_GroupByChannel, &electrophoretic, nullptr, nullptr);
		DAQmxWriteAnalogF64(task1, 1, true, 10.0, DAQmx_Val_GroupByChannel, &voltage, nullptr, nullptr);
		grphValues.push_back(voltage);
		grphVa.push_back(voltage);

		std::stringstream ss;
		ss << "Running Voltage: " << voltage<<"V";
		std::string voltageStr = ss.str();

		std::stringstream sis;
		sis << "EPD Voltage: " << electrophoretic<<"V";
		std::string veel = sis.str();

		auto currentTime = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> elapsed = currentTime - startTime;
		double elapsedTime = elapsed.count();

		std::stringstream timeSS;
		timeSS << "Time : " << elapsedTime << " s ";
		std::string timeStr = timeSS.str();

		std::stringstream totime;
		totime << "Time of Hmax : " << etime << " s ";
		std::string maxtime = totime.str();

		cv::Mat infoA = fullScreenImage(cv::Rect(0, 0, fwidth * 0.5, fheight * 0.5));
		cv::resize(dframe, dframe, infoA.size());
		cv::addWeighted(dframe, 1.0, dframe, 0, 0, infoA);

		

		cv::Mat infoAreas = fullScreenImage(cv::Rect(fwidth / 2, 0, fwidth / 2, fheight / 2));
		infoAreas.setTo(white);

		cv::Mat graapp = fullScreenImage(cv::Rect(0, fheight / 2, fwidth, fheight / 4));
		infoAreas.setTo(white);

		cv::Mat graappix = fullScreenImage(cv::Rect(0, (3 * fheight / 4), fwidth, fheight / 4));
		infoAreas.setTo(white);
		pixData.push_back(contrast);
		/*if (pixData.size() > (fullScreenImage.rows - 30)) {
			pixData.pop_front();
		}*/
		allgraph(graapp, pixData,1);
		allgraph(graappix, grphVa,pztvolt);

		int y = 35;
		drawText(infoAreas, timeStr, 5, y, 0.5, red, 2);
		y += 35;
		drawText(infoAreas, pztStr, 5, y, 0.5, red, 1);
		y += 35;
		drawText(infoAreas, voltageStr, 5, y, 0.5, red, 1);
		y += 35;
		drawText(infoAreas, status, 5, y, 0.5, red, 1);
		y += 35;
		drawText(infoAreas, maxtime, 5, y, 0.5, red, 1);
		y += 35;
		drawText(infoAreas, constr, 5, y, 0.5, red, 1);
		y += 35;
		drawText(infoAreas, veel, 5, y, 0.5, red, 1);
		y += 35;
		drawText(infoAreas, thrr, 5, y, 0.5, red, 1);
		y += 35;
		drawText(infoAreas, ima, 5, y, 0.5, black, 1);

		cv::imshow("Status of deposition", fullScreenImage);
		cv::moveWindow("Status of deposition", 0, 0);

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
	std::stringstream yy1;
	yy1 << text << " ";
	std::string tr1 = yy1.str();

	std::stringstream yy2;
	yy2 << text/2 << " ";
	std::string tr2 = yy2.str();

	std::stringstream yy3;
	yy3 << text-text <<" ";
	std::string tr3 = yy3.str();

	drawText(graphArea, tr1, 10, graphArea.rows*0.15, 0.5, red, 2);
	drawText(graphArea, tr2, 7, graphArea.rows*0.5, 0.5, red, 1);
	drawText(graphArea, tr3, 10, graphArea.rows*0.95, 0.5, red, 2);
}

void Deposition::drawXAxis(cv::Mat& graphArea, const cv::Scalar& color) {
	double y = (thContrast / 1) * (graphArea.rows * 0.8) + 10;
	DrawDashedLine(graphArea, cv::Point(30, y), cv::Point(graphArea.cols, y), red, 2, "", 10);
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