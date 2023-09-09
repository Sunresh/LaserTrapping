#include "Only.h"

Depositi::Depositi() {
	std::vector<UserPreferences> storedPreferences = readFromCSV(filename);
	for (const UserPreferences& prefs : storedPreferences) {
		pztvolt = prefs.voltage;
		thContrast = prefs.threshold;
		timefordeposition=prefs.time;
	}
	camm.open(0);
	if (!camm.isOpened()) {
		return;
	}
}

Depositi::~Depositi() {
	if (camm.isOpened()) {
		camm.release();
		cout << "Cam is closed now.";
	}
}

void Depositi::camra() {
	if (!camm.isOpened()) {
		return;
	}
	std::deque<double> graphValues;
	while (true) {
		cv::Mat frame;
		camm >> frame;
		if (frame.empty()) {
			break;
		}
		cv::rectangle(frame, pt1, pt2, red, 1);
		cv::Rect roiRect(x1, y1, roiw, roih); // (x, y, width, height)
		cv::Mat grayColorRect = frame(roiRect).clone();
		double contrast = calculateContrast(grayColorRect);
		graphValues.push_back(contrast);
		if (graphValues.size() > (frame.cols - 3 * padding) ) {
			graphValues.pop_front();
		}
		drawGraphC(frame, graphValues);
		cv::imshow(appName, frame);
		cv::moveWindow(appName, 0, 0);
		char key = cv::waitKey(1);
		if (key == 'q' || key == 'Q' || key == 21) {
			camm.release();
			cv::destroyAllWindows();
			diamond.application();
			break;
		}
	}
}

double Depositi::calculateContrast(const cv::Mat& frame) {
	int width = frame.cols;
	int height = frame.rows;
	cv::Mat grayFrame;
	cv::cvtColor(frame, grayFrame, cv::COLOR_BGR2GRAY);
	double sumPij = 0.0;
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			sumPij += grayFrame.at<uchar>(y, x);
		}
	}
	double contrast = sumPij / (255.0 * width * height);
	return contrast;
}

void Depositi::drawGraph(cv::Mat& frame, const std::deque<double>& graphValues) {
	cv::Mat graphArea = frame(cv::Rect(0, 0, frame.cols, frame.rows * 0.25));
	graphArea.setTo(white);
	drawGraphLines(graphArea, graphValues);
	drawYAxisValues(graphArea, graphHeight, black, std::to_string(pztvolt));
	drawXAxis(graphArea, graphHeight, black);
}
void Depositi::drawGraphC(cv::Mat& frame, const std::deque<double>& graphValues) {
	cv::Mat graphArea = frame(cv::Rect(0, 0, frame.cols, frame.rows * 0.25));
	graphArea.setTo(white);
	drawCLines(graphArea, graphValues);
	drawYAxisValues(graphArea, graphHeight, black, std::to_string(pztvolt));
	drawXAxis(graphArea, graphHeight, black);
}

//void Depositi::drawGraphLines(cv::Mat& graphArea, const std::deque<double>& graphValues) {
//	cv::Point startPoint(30, 0);
//	int height = graphArea.rows;
//	int width = graphArea.cols;
//	double amplitude = 100;
//	for (int i = 0; i < graphValues.size(); ++i) {
//		double y = height * graphValues[i];
//		y = (y + amplitude) / ( 2 * amplitude) * (height - 1);
//		cv::Point endPoint(i+30, height - static_cast<int>(y));
//		line(graphArea, startPoint, endPoint, black, 1);
//		startPoint = endPoint;
//		cout << y << endl;
//	}
//}


void Depositi::drawGraphLines(cv::Mat& graphArea, const std::deque<double>& graphValues) {
	const int startPointX = 30; // Start X-coordinate
	const int lineThickness = 1; // Line thickness
	if (graphValues.empty()) {
		return;
	}
	int height = graphArea.rows;
	int width = graphArea.cols;
	cv::Point startPoint(startPointX, 30);
	for (int i = 0; i < graphValues.size(); ++i) {
		double y = graphValues[i] * (height-54);
		cv::Point endPoint(i + startPointX, height - static_cast<int>(y));
		line(graphArea, startPoint, endPoint, cv::Scalar(0, 0, 0), lineThickness);
		startPoint = endPoint;
	}
}

void Depositi::drawCLines(cv::Mat& graphArea, const std::deque<double>& graphValues) {
	const int startPointX = 30; // Start X-coordinate
	const int lineThickness = 1; // Line thickness
	if (graphValues.empty()) {
		return;
	}
	int height = graphArea.rows;
	int width = graphArea.cols;
	cv::Point startPoint(startPointX, 30);
	for (int i = 0; i < graphValues.size(); ++i) {
		double y = graphValues[i] * (height + 27);
		cv::Point endPoint(i + startPointX, height - static_cast<int>(y));
		line(graphArea, startPoint, endPoint, cv::Scalar(0, 0, 0), lineThickness);
		startPoint = endPoint;
	}
}


void Depositi::drawYAxisValues(cv::Mat& graphArea, int graphHeight,const cv::Scalar& color, const std::string& text) {
	putText(graphArea, text, cv::Point(0, 10), cv::FONT_HERSHEY_PLAIN, 0.7, color, 1);
	putText(graphArea, "0", cv::Point(0, graphArea.rows-10), cv::FONT_HERSHEY_PLAIN, 0.7, color, 1);
}

void Depositi::drawXAxis(cv::Mat& graphArea, int graphHeight, const cv::Scalar& color) {
	line(graphArea, cv::Point(30, 10), cv::Point(30, graphArea.rows - 10), color, 1);//verticle
	//line(graphArea, cv::Point(30, graphHeight - 10), cv::Point(graphArea.cols, graphHeight - 10), color, 1);//horizontal
}

void Depositi::writeContrastToCSV(const std::string& filename, const std::vector<double>& contrastData) {
	std::ofstream outFile(filename);
	if (!outFile.is_open()) {
		std::cerr << "Error opening file for writing." << std::endl;
		return;
	}
	outFile << "Serial Number,Contrast" << std::endl;
	for (int i = 0; i < contrastData.size(); ++i) {
		outFile << i + 1 << "," << contrastData[i] << std::endl;
	}
	outFile.close();
}
void Depositi::dodeposition() {
	if (!camm.isOpened()) {
		return;
	}
	DAQmxCreateTask("", &task1);
	DAQmxCreateTask("", &task2);
	DAQmxCreateAOVoltageChan(task1, dev1, "ao_channel", 0.0, 5.0, DAQmx_Val_Volts, nullptr);
	DAQmxCreateAOVoltageChan(task2, dev0, "ao_channel", 0.0, 5.0, DAQmx_Val_Volts, nullptr);
	DAQmxCfgSampClkTiming(task1, "", 10.0, DAQmx_Val_Rising, DAQmx_Val_ContSamps, 1);
	DAQmxCfgSampClkTiming(task2, "", 10.0, DAQmx_Val_Rising, DAQmx_Val_ContSamps, 1);

	double durationInSeconds = timefordeposition;
	int numSteps = durationInSeconds * 100;
	double startVolt = 0.0;
	double endVolt = pztvolt;

	std::stringstream pztvoltage;
	pztvoltage << "Applied Voltage: " << endVolt;
	std::string pztStr = pztvoltage.str();

	auto now = std::chrono::system_clock::now();
	auto time_t_now = std::chrono::system_clock::to_time_t(now);
	struct tm timeinfo;
	localtime_s(&timeinfo, &time_t_now); // Platform-specific, for Windows
	char filename[80];
	char filena[80];
	char ima[80];
	char filee[80];
	strftime(filename, sizeof(filename), "%Y-%m-%d_%H-%M-%S.jpg", &timeinfo);
	strftime(filena, sizeof(filena), "%Y-%m-%d_%H-%M-%S.csv", &timeinfo);
	strftime(ima, sizeof(ima), "%Y-%m-%d_%H-%M-%S", &timeinfo);
	strftime(filee, sizeof(filee), "%Y-%m-%d_%H-%M-%S_vol.csv", &timeinfo);

	double stepDuration = durationInSeconds / numSteps;
	const int graphWidth = 400;
	const int graphHeight = 400;
	auto startTime = std::chrono::high_resolution_clock::now();
	double etime = 0;
	bool updated = false;
	bool isIncrease = true;
	std::deque<double> lla;
	double voltage = startVolt;
	cv::Mat dframe, grayColorRect;
	std::vector<double> contrastData;
	std::vector<double> grphValues;
	std::deque<double> grphVa;

	for (int step = 0; step <= numSteps * 2; step++) {
		camm >> dframe;
		if (dframe.empty()) {
			break;
		}
		cv::Rect roiRect(x1, y1, roiw, roih); // (x, y, width, height)
		grayColorRect = dframe(roiRect).clone();
		double contrast = calculateContrast(grayColorRect);
		contrastData.push_back(contrast);
		std::stringstream con;
		con << "Constrast: " << contrast;
		std::string constr = con.str();

		cv::Mat frame(graphHeight, graphWidth, CV_8UC4, cv::Scalar(0, 0, 0, 0.5));
		string status;
		if (step == 0) {
			voltage += endVolt / numSteps;
			double volt = 2.0;
			status = "Starting..";
			cout << status << endl;
			DAQmxWriteAnalogF64(task2, 1, true, 10.0, DAQmx_Val_GroupByChannel, &volt, nullptr, nullptr);
		}
		else if (isIncrease) {
			if (voltage < endVolt && contrast>thContrast) {
				status = "Deposition is going on..";
				voltage += endVolt / numSteps;
				double volt = 2.0;
				DAQmxWriteAnalogF64(task2, 1, true, 10.0, DAQmx_Val_GroupByChannel, &volt, nullptr, nullptr);
			}
			if (voltage < endVolt && contrast < thContrast && voltage > 0) {
				status = "Stage up..";
				numSteps += 3;
				voltage -= endVolt / numSteps;
				double volt = 0.0;
				DAQmxWriteAnalogF64(task2, 1, true, 10.0, DAQmx_Val_GroupByChannel, &volt, nullptr, nullptr);

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
				double volt = 0.0;
				DAQmxWriteAnalogF64(task2, 1, true, 10.0, DAQmx_Val_GroupByChannel, &volt, nullptr, nullptr);

			}
		}

		if (!isIncrease) {
			status = "Deposition finished..";
			voltage -= endVolt / numSteps;
		}

		DAQmxWriteAnalogF64(task1, 1, true, 10.0, DAQmx_Val_GroupByChannel, &voltage, nullptr, nullptr);
		grphValues.push_back(voltage);
		grphVa.push_back(voltage);

		drawGraph(frame, grphVa);


		std::stringstream ss;
		ss << "Voltage: " << voltage;
		std::string voltageStr = ss.str();

		auto currentTime = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> elapsed = currentTime - startTime;
		double elapsedTime = elapsed.count();

		// Convert the time to a string for time display
		std::stringstream timeSS;
		timeSS << "Time : " << elapsedTime << " s ";
		std::string timeStr = timeSS.str();

		// Convert the time to a string for time display
		std::stringstream totime;
		totime << "Time of H : " << etime << " s ";
		std::string maxtime = totime.str();

		int y = 140;
		drawText(frame, timeStr, 5, y, 1.0, red, 2);
		y += 40;
		drawText(frame, pztStr, 5, y, 1.0, red, 1);
		y += 40;
		drawText(frame, voltageStr, 5, y, 1.0, red, 1);
		y += 40;
		drawText(frame, status, 5, y, 1.0, red, 1);
		y += 40;
		drawText(frame, maxtime, 5, y, 1.0, red, 1);
		y += 40;
		drawText(frame, constr, 5, y, 1.0, red, 1);

		cv::imshow("Status of deposition", frame);
		cv::moveWindow("Status of deposition", 1000, 0);
		char key = cv::waitKey(1);
		if (key == 'q' || key == 21) {
			diamond.application();
			camm.release();
			cv::destroyAllWindows();
			app();
			break;
		}
		if (step == numSteps * 2) {
			std::string imawa = std::string(ima);
			cv::putText(frame, imawa, cv::Point(5, 280), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);
			std::string fullFilename = std::string(filename);
			std::string fullFilena = std::string(filena);
			std::string fullFile = std::string(filee);
			cv::imwrite(fullFilename, frame);
			writeContrastToCSV(fullFilena, contrastData);
			writeContrastToCSV(fullFile, grphValues);
			cv::destroyWindow("Status of deposition");
			return app();
		}
	}

	DAQmxClearTask(task1);
	DAQmxClearTask(task2);
}
void Depositi::cAndD() {
	if (!camm.isOpened()) {
		return;
	}
	DAQmxCreateTask("", &task1);
	DAQmxCreateTask("", &task2);
	DAQmxCreateAOVoltageChan(task1, dev1, "ao_channel", 0.0, 5.0, DAQmx_Val_Volts, nullptr);
	DAQmxCreateAOVoltageChan(task2, dev0, "ao_channel", 0.0, 5.0, DAQmx_Val_Volts, nullptr);
	DAQmxCfgSampClkTiming(task1, "", 10.0, DAQmx_Val_Rising, DAQmx_Val_ContSamps, 1);
	DAQmxCfgSampClkTiming(task2, "", 10.0, DAQmx_Val_Rising, DAQmx_Val_ContSamps, 1);

	double durationInSeconds = timefordeposition;
	int numSteps = durationInSeconds * 100;
	double startVolt = 0.0;
	double endVolt = pztvolt;


	std::stringstream pztvoltage;
	pztvoltage << "Applied Voltage: " << endVolt;
	std::string pztStr = pztvoltage.str();

	auto now = std::chrono::system_clock::now();
	auto time_t_now = std::chrono::system_clock::to_time_t(now);
	struct tm timeinfo;
	localtime_s(&timeinfo, &time_t_now); // Platform-specific, for Windows
	char filename[80];
	char filena[80];
	char ima[80];
	char filee[80];
	strftime(filename, sizeof(filename), "%Y-%m-%d_%H-%M-%S.jpg", &timeinfo);
	strftime(filena, sizeof(filena), "%Y-%m-%d_%H-%M-%S.csv", &timeinfo);
	strftime(ima, sizeof(ima), "%Y-%m-%d_%H-%M-%S", &timeinfo);
	strftime(filee, sizeof(filee), "%Y-%m-%d_%H-%M-%S_vol.csv", &timeinfo);

	double stepDuration = durationInSeconds / numSteps;
	const int graphWidth = 400;
	const int graphHeight = 400;
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

	for (int step = 0; step <= numSteps * 2; step++) {
		camm >> dframe;
		if (dframe.empty()) {
			break;
		}
		cv::rectangle(dframe, pt1, pt2, red, 1);
		cv::Rect roiRect(x1, y1, roiw, roih); // (x, y, width, height)
		grayColorRect = dframe(roiRect).clone();
		double contrast = calculateContrast(grayColorRect);
		contrastData.push_back(contrast);

		std::stringstream con;
		con << "Constrast: " << contrast;
		std::string constr = con.str();

		cv::Mat frame(graphHeight, graphWidth, CV_8UC4, cv::Scalar(0, 0, 0, 0.5));
		string status;
		if (step == 0) {
			voltage += endVolt / numSteps;
			status = "Starting..";
			electrophoretic = 2.0;
		}
		else if (isIncrease) {
			if (voltage < endVolt && contrast>thContrast) {
				status = "Deposition is going on..";
				voltage += endVolt / numSteps;
				electrophoretic = 2.0;
			}
			if (voltage < endVolt && contrast < thContrast && voltage > 0) {
				status = "Stage up..";
				numSteps += 3;
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

		if (!isIncrease) {
			status = "Deposition finished..";
			voltage -= endVolt / numSteps;
			electrophoretic = 0.0;
		}
		DAQmxWriteAnalogF64(task2, 1, true, 10.0, DAQmx_Val_GroupByChannel, &electrophoretic, nullptr, nullptr);
		DAQmxWriteAnalogF64(task1, 1, true, 10.0, DAQmx_Val_GroupByChannel, &voltage, nullptr, nullptr);
		grphValues.push_back(voltage);
		grphVa.push_back(voltage);

		std::stringstream ss;
		ss << "Voltage: " << voltage;
		std::string voltageStr = ss.str();

		auto currentTime = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> elapsed = currentTime - startTime;
		double elapsedTime = elapsed.count();

		// Convert the time to a string for time display
		std::stringstream timeSS;
		timeSS << "Time : " << elapsedTime << " s ";
		std::string timeStr = timeSS.str();

		// Convert the time to a string for time display
		std::stringstream totime;
		totime << "Time of H : " << etime << " s ";
		std::string maxtime = totime.str();
		

		cv::Mat fullScreenImage(700, 1100, CV_8UC3, cv::Scalar(255, 255, 255)); // Red background
		int fwidth = fullScreenImage.cols;//dframe
		int fheight = fullScreenImage.rows;
		cv::Mat infoA = fullScreenImage(cv::Rect(0, 0, fwidth / 2, fheight / 2));
		infoA.setTo(red);
		cv::resize(dframe, dframe, infoA.size());
		double alpha = 1.0;
		cv::addWeighted(dframe, alpha, infoA, 1.0 - alpha, 0, infoA);

		pixData.push_back(contrast);
		if (pixData.size() > (fullScreenImage.cols - 3 * padding)) {
			pixData.pop_front();
		}

		cv::Mat infoAreas = fullScreenImage(cv::Rect(fwidth / 2, 0, fwidth / 2, fheight / 2));
		infoAreas.setTo(white);

		cv::Mat graapp = fullScreenImage(cv::Rect(0, fheight / 2, fwidth, fheight / 4));
		infoAreas.setTo(white);
		
		cv::Mat graappix = fullScreenImage(cv::Rect(0, ( 3* fheight / 4), fwidth, fheight / 4));
		infoAreas.setTo(white);

		allgraphc(graapp, pixData);
		allgraph(graappix, grphVa);

		int y = 40;
		drawText(infoAreas, timeStr, 5, y, 1.0, red, 2);
		y += 40;
		drawText(infoAreas, pztStr, 5, y, 1.0, red, 1);
		y += 40;
		drawText(infoAreas, voltageStr, 5, y, 1.0, red, 1);
		y += 40;
		drawText(infoAreas, status, 5, y, 1.0, red, 1);
		y += 40;
		drawText(infoAreas, maxtime, 5, y, 1.0, red, 1);
		y += 40;
		drawText(infoAreas, constr, 5, y, 1.0, red, 1);

		


		

		cv::imshow("Status of deposition", fullScreenImage);
		cv::moveWindow("Status of deposition", 0, 0);
		
		char key = cv::waitKey(1);
		if (key == 'q' || key == 21) {
			diamond.application();
			camm.release();
			cv::destroyAllWindows();
			app();
			break;
		}
		if (step == numSteps * 2) {
			std::string imawa = std::string(ima);
			cv::putText(frame, imawa, cv::Point(5, 280), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);
			std::string fullFilename = std::string(filename);
			std::string fullFilena = std::string(filena);
			std::string fullFile = std::string(filee);
			cv::imwrite(fullFilename, frame);
			writeContrastToCSV(fullFilena, contrastData);
			writeContrastToCSV(fullFile, grphValues);
			cv::destroyWindow("Status of deposition");
			return app();
		}
	}

	DAQmxClearTask(task1);
	DAQmxClearTask(task2);
}
void Depositi::allgraph(cv::Mat& frame, const std::deque<double>& graphValues) {
	const int startPointX = 30; // Start X-coordinate
	const int lineThickness = 1; // Line thickness
	if (graphValues.empty()) {
		return;
	}
	int height = frame.rows;
	int width = frame.cols;
	cv::Point startPoint(startPointX, 30);
	for (int i = 0; i < graphValues.size(); ++i) {
		double y = (graphValues[i]/ pztvolt) * (height);
		cv::Point endPoint(i + startPointX, height - static_cast<int>(y));
		line(frame, startPoint, endPoint, cv::Scalar(0, 0, 0), lineThickness);
		startPoint = endPoint;
	}
	drawYAxisValues(frame, graphHeight, black, std::to_string(pztvolt));
	drawXAxis(frame, graphHeight, black);
}
void Depositi::allgraphc(cv::Mat& frame, const std::deque<double>& graphValues) {
	const int startPointX = 30; // Start X-coordinate
	const int lineThickness = 1; // Line thickness
	if (graphValues.empty()) {
		return;
	}
	int height = frame.rows;
	int width = frame.cols;
	cv::Point startPoint(startPointX, 30);
	for (int i = 0; i < graphValues.size(); ++i) {
		double y = graphValues[i] * (height);
		cv::Point endPoint(i + startPointX, height - static_cast<int>(y));
		line(frame, startPoint, endPoint, cv::Scalar(0, 0, 0), lineThickness);
		startPoint = endPoint;
	}
	drawYAxisValues(frame, graphHeight, black, "1");
	drawXAxis(frame, graphHeight, black);
}
void Depositi::drawText(cv::Mat& frame, const std::string& text, int x, int y, int fontSize, const cv::Scalar& color, int thickness = 1) {
	cv::putText(frame, text, cv::Point(x, y), cv::FONT_HERSHEY_SIMPLEX, fontSize, color, thickness);
}

void Depositi::drawRectangle(cv::Mat& frame, int x1, int y1, int x2, int y2, const cv::Scalar& color, int thickness = 1) {
	cv::rectangle(frame, cv::Point(x1, y1), cv::Point(x2, y2), color, thickness);
}


void Depositi::app() {
	system("cls");
	std::vector<UserPreferences> storedPreferences = readFromCSV(filename);

	// Display stored preferences
	for (const UserPreferences& prefs : storedPreferences) {
		std::cout << "\t\t" << "PZT Voltage: " << prefs.voltage << "\n";
		std::cout << "\t\t" << "Pixel Contrast: " << prefs.threshold << "\n";
		std::cout << "\t\t" << "Time for velocity: " << prefs.time << "\n";
	}

	string str[] = {
		" ",
		"'e' for edit, you want to change the above parameter.",
		" ",
		"c to start ccd camera.",
		"m to start deposition.",
		"b to start camera and deposition.",
		" ",
	};
	for (string i : str) {
		if (i.size() < 6) {
			cout << "\t\t" << i << endl;
		}
		else {
			cout << "\t" << "Press " << i << endl;
		}
	}
	char key;
	char a = 'a';
	while (true) {
		key = _getch();
		if (key == 'e' || key == 'E') {
			getPrefToCSV(filename);
		}
		if (key == 'd' || key == 'D') {
			Depositi dia;
			std::thread ddaa(&Depositi::dodeposition, &dia);
			ddaa.join();
		}
		if (key == 'x' || key == 'X') {
			Depositi dia;
			std::thread ddaa(&Depositi::cAndD, &dia);
			ddaa.join();
		}
		if (key == 'c' || key == 'C') {
			Depositi tdd;
			std::thread threadA(&Depositi::camra, &tdd);
			threadA.join();
		}
		if (key == 'b' || key == 'B') {
			Depositi tdd;
			Depositi d;
			std::thread threadA(&Depositi::camra, &tdd);
			std::thread threadDeposition(&Depositi::dodeposition, &d);
			threadA.join();
			threadDeposition.join();

		}
		else {
			std::cout << "Exiting program." << std::endl;
			break;
		}
	}

}
