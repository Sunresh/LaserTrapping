#include <deque>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include "preference.h"
#include <conio.h>

using namespace std;
cv::Point POINT1, POINT2;
int radius;
int XaxisX1, YaxisY1;
double VOLTAGE, BRIGHTNESS;
int TTIME;
cv::Scalar red, white, black,green;
std::string commonPath;
char* dev0;
char* dev1;
int CAMERA;
std::string DesktopFolder;
std::string LAST_VOLT_FILE;


Pref::Pref() : height(),threshold(),time() {
	PWSTR path;
	if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Desktop, 0, NULL, &path))) {
		DesktopFolder = std::string(path, path + wcslen(path));
		CoTaskMemFree(path);
	}
	else {
		std::wcerr << L"Failed to get the desktop path" << std::endl;
	}
	red = cv::Scalar(0, 0, 255);
	white = cv::Scalar(255, 255, 255);
	black = cv::Scalar(0, 0, 0);
	green = cv::Scalar(0, 255, 0);
	commonPath = DesktopFolder + "/allout/";
	dev0 = "Dev2/ao0";
	dev1 = "Dev2/ao1";
	LAST_VOLT_FILE = "lastvolt.csv";
	UserPreferences prefs;
	if (!loadCSV(PREF_FILE, prefs)) {
		std::cerr << "No preferences found or error reading preferences. Creating with default values." << std::endl;
	}
	else {
		loadCSV(PREF_FILE, prefs);
	}
	VOLTAGE = prefs.height / 6;
	BRIGHTNESS = prefs.threshold;
	TTIME = prefs.time;
	radius = prefs.radius;
	XaxisX1 = prefs.left;
	YaxisY1 = prefs.top;
	POINT1 = cv::Point(XaxisX1, YaxisY1);
	POINT2 = cv::Point(XaxisX1 + radius, YaxisY1 + radius);
	CAMERA = prefs.camera;
}

void Pref::setHeight(int newHeight) {
	height = newHeight;
}

int Pref::getHeight() const {
	return height;
}

bool Pref::isNumeric(const std::string& str) {
	for (char c : str) {
		if (!std::isdigit(c) && c != '.' && c != '-') {
			return false;
		}
	}
	return true;
}
void Pref::getUserInput(const std::string& fieldName, int& field) {
	std::string input;
	std::cout << "Enter " << fieldName << ": ";
	std::cin >> input;
	if (!isNumeric(input)) {
		std::cerr << "Error: " << fieldName << " must be a numeric value." << std::endl;
		return;
	}
	field = std::stoi(input);
}
void Pref::getUserInput(const std::string& fieldName, double& field) {
	std::string input;
	std::cout << "Enter " << fieldName << ": ";
	std::cin >> input;
	if (!isNumeric(input)) {
		std::cerr << "Error: " << fieldName << " must be a numeric value." << std::endl;
		return;
	}
	field = std::stod(input);
}
void Pref::getDirectV(const std::string& fieldName, double& field, const string& val) {
	field = std::stod(val);
}

void Pref::allentry() {
	UserPreferences userPrefs;
	std::cerr << "No preferences found or error reading preferences. Creating with default values." << std::endl;
	getUserInput("Pillar height", userPrefs.height);
	getUserInput("threshold value for laser spot contrast", userPrefs.threshold);
	getUserInput("time of deposition", userPrefs.time);
	getUserInput("spot from left", userPrefs.left);
	getUserInput("spot from top", userPrefs.top);
	saveCSV(PREF_FILE, userPrefs);
}
void Pref::saveCSV(const std::string& filename, const UserPreferences& userPrefs) {
	std::ofstream outFile(filename);
	if (!outFile.is_open()) {
		std::cerr << "Error opening file for writing." << std::endl;
		return;
	}
	// Write the preferences to the file with commas
	outFile
		<< userPrefs.height << ","
		<< userPrefs.threshold << ","
		<< userPrefs.time << ","
		<< userPrefs.left << ","
		<< userPrefs.top << ","
		<< userPrefs.camera << ","
		<< userPrefs.radius <<endl;
	outFile.close();
}

bool Pref::loadCSV(const std::string& filename, UserPreferences& userPrefs) {
	std::ifstream inFile(filename);
	if (!inFile.is_open()) {
		allentry();
		return false; // File doesn't exist or cannot be opened; use default values
	}
	std::string line;
	std::getline(inFile, line);
	std::istringstream ss(line);
	std::string value;
	getline(ss, value, ',');
	userPrefs.height = std::stoi(value);
	getline(ss, value, ',');
	userPrefs.threshold = std::stod(value);
	getline(ss, value, ',');
	userPrefs.time = std::stoi(value);
	getline(ss, value, ',');
	userPrefs.left = std::stoi(value);
	getline(ss, value, ',');
	userPrefs.top = std::stoi(value);
	getline(ss, value, ',');
	userPrefs.camera = std::stod(value);
	getline(ss, value, ',');
	userPrefs.radius = std::stod(value);

	inFile.close();
	return true;
}

void Pref::app(const std::string& filename) {
	UserPreferences userPrefs;
	if (!loadCSV(filename, userPrefs)) {
		std::cerr << "No preferences found or error reading preferences. Creating with default values." << std::endl;
	}
	else {
		loadCSV(filename, userPrefs);
	}

	char key;
	while (true) {
		startscreen();
		helpscreen();
		key = _getch(); // Use getchar to read a character
		switch (key) {
		case 'z':
			getUserInput("Pillar Height", userPrefs.height);
			break;
		case 'x':
			getUserInput("threshold value for laser spot contrast", userPrefs.threshold);
			break;
		case 'c':
			getUserInput("time of deposition", userPrefs.time);
			break;
		case 'l':
			getUserInput("spot from left", userPrefs.left);
			break;
		case 't':
			getUserInput("spot from top", userPrefs.top);
			break;
		case 'r':
			getUserInput("spot radius", userPrefs.radius);
			break;
		case 'w':
			getDirectV("Webcam", userPrefs.camera, "0");
			break;
		case 'W':
			getDirectV("CCD cam", userPrefs.camera, "1");
			break;
		case 'q':
			break;
		default:
			continue; // Continue the loop for unknown keys
		}
		saveCSV(filename, userPrefs);
		break;
	}
}

void Pref::startscreen() {
	UserPreferences userPrefs;
	if (!loadCSV(PREF_FILE, userPrefs)) {
		std::cerr << "No preferences found or error reading preferences. Creating with default values." << std::endl;
	}
	else {
		loadCSV(PREF_FILE, userPrefs);
	}
//	system("cls");
	std::cout << "\t\t" << std::string(48, '_') << std::endl;
	std::cout << "\t\t" << "|" << std::string(46, ' ') << "|" << std::endl;
	std::cout << "\t\t" << "|  z. Pillar Height(micro-m): " << userPrefs.height << std::string(15, ' ') << "|" << std::endl;
	std::cout << "\t\t" << "|  x. Contrast:               " << userPrefs.threshold << std::string(16, ' ') << "|" << std::endl;
	std::cout << "\t\t" << "|  c. Time for velocity:      " << userPrefs.time << std::string(16, ' ') << "|" << std::endl;
	std::cout << "\t\t" << "|  l. left:                   " << userPrefs.left << std::string(14, ' ') << "|" << std::endl;
	std::cout << "\t\t" << "|  t. top:                    " << userPrefs.top << std::string(14, ' ') << "|" << std::endl;
	std::cout << "\t\t" << "|  r. radius:                 " << userPrefs.radius << std::string(14, ' ') << "|" << std::endl;
	if (userPrefs.camera == 0) {
		std::cout << "\t\t" << "|  w. Camera:                 " << "Webcam" << std::string(11, ' ') << "|" << std::endl;
	}
	if (userPrefs.camera == 1) {
		std::cout << "\t\t" << "|  W. Camera:                 " << "CCD Camera" << std::string(7, ' ') << "|" << std::endl;
	}
	std::cout << "\t\t" << "|" << std::string(46, ' ') << "|" << std::endl;
	std::cout << "\t\t" << std::string(48, '-') << std::endl;
}
void Pref::helpscreen() {
	string str[] = {
		" ",
		"z ---> height of pillar",
		"x ---> brightness threshold",
		"c ---> Time",
		"l ---> x1 of select area",
		"t ---> y1 of select area",
		"r ---> radius",
		"w ---> Laptop Webcam ",
		"W ---> CCD CAMERA",
		"q ---> BACK <---",
		" ",
	};
	for (string i : str) {
		if (i.size() < 6) {
			cout << "\t\t\t" << i << endl;
		}
		else {
			cout << "\t\t-> " << "Press " << i << endl;
		}
	}
}
void Pref::menu() {
	string str[] = {
		" ",
		"e for edit, you want to change the above parameter.",
		" ",
		"c to start ccd camera.",
		"k to start deposition.",
		" ",
	};
	for (string i : str) {
		if (i.size() < 6) {
			cout << "\t\t\t" << i << endl;
		}
		else {
			cout << "\t\t-> " << "Press " << i << endl;
		}
	}
}
std::string Pref::double2string(const double& value, const std::string& stri) {
	std::stringstream sis;
	sis << stri << value << " ";
	std::string thrr = sis.str();
	return thrr;
}

void Pref::simpleCSVsave(std::string& filename,double value) {
	std::string fileout = commonPath + filename;
	std::ofstream file(fileout);
	if (!file.is_open()) {
		std::cerr << "Failed to open the file." << std::endl;
		return;
	}
	file << value << "\n";
	file.close();
}
std::string Pref::simpleCSVread(std::string& filename) {
	std::string fileout = commonPath + filename;
	std::ifstream file(fileout);
	if (!file.is_open()) {
		std::cerr << "Failed to open the file." << std::endl;
		return "the";
	}
	std::string line;
	if (std::getline(file, line)) {
		// Assuming the CSV file contains a single value per line
		try {
			std::string value = line;
			return value;
		}
		catch (const std::invalid_argument& e) {
			std::cerr << "Failed to convert value from CSV: " << e.what() << std::endl;
			return e.what();
		}
		catch (const std::out_of_range& e) {
			std::cerr << "Value from CSV out of range: " << e.what() << std::endl;
			return e.what();
		}
	}
	
}

void Pref::slowlyslowly(std::string& filename) {
	std::string gett = simpleCSVread(filename);
	double value = std::stod(gett);
	const char* dev0 = "Dev2/ao0";
	const char* dev1 = "Dev2/ao1";
	double v1 = 0.0;
	TaskHandle task1 = nullptr;
	TaskHandle task2 = nullptr;
	DAQmxCreateTask("", &task1);
	DAQmxCreateTask("", &task2);
	DAQmxCreateAOVoltageChan(task1, dev1, "ao_channel", 0.0, 5.0, DAQmx_Val_Volts, nullptr);
	DAQmxCreateAOVoltageChan(task2, dev0, "ao_channel", 0.0, 5.0, DAQmx_Val_Volts, nullptr);
	DAQmxCfgSampClkTiming(task1, "", 10.0, DAQmx_Val_Rising, DAQmx_Val_ContSamps, 1);
	DAQmxCfgSampClkTiming(task2, "", 10.0, DAQmx_Val_Rising, DAQmx_Val_ContSamps, 1);
	DAQmxWriteAnalogF64(task2, 1, true, 10.0, DAQmx_Val_GroupByChannel, &v1, nullptr, nullptr);
	if (value != 0) {
		for (int i = 1; i < 100; i++) {
			value -= value / (100);
			DAQmxWriteAnalogF64(task1, 1, true, 10.0, DAQmx_Val_GroupByChannel, &value, nullptr, nullptr);
		}
	}
	DAQmxClearTask(task1);
	DAQmxClearTask(task2);
}

void Pref::createDefaultFile(const std::string& filename) {
    std::ofstream outFile(filename);

    if (!outFile.is_open()) {
        std::cerr << "Error: Unable to create default file '" << filename << "'" << std::endl;
        return;
    }

    // Write default values to the file
    outFile << "100,0.5,10,20,30,1.0"; // Modify this to include your default values
    outFile.close();

    std::cout << "Default file '" << filename << "' created successfully." << std::endl;
}