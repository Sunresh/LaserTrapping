#include <deque>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include "preference.h"
#include <conio.h>

using namespace std;
cv::Point POINT1;
cv::Point POINT2;
int radius;
int XaxisX1;
int YaxisY1;
double VOLTAGE;
double BRIGHTNESS;
int TTIME;
cv::Scalar red;
cv::Scalar white;
cv::Scalar black;
std::string commonPath;
char* dev0;
char* dev1;

Pref::Pref() {

	red = cv::Scalar(0, 0, 255);
	white = cv::Scalar(255, 255, 255);
	black = cv::Scalar(0, 0, 0);
	commonPath = "C:/Users/nares/Desktop/allout/";
	dev0 = "Dev2/ao0";
	dev1 = "Dev2/ao1";

	std::vector<UserPreferences> storedPreferences = readFromCSV(PREF_FILE);
	for (const UserPreferences& prefs : storedPreferences) {
		VOLTAGE = prefs.height / 6;
		BRIGHTNESS = prefs.threshold;
		TTIME = prefs.time;
		XaxisX1 = prefs.left;
		YaxisY1 = prefs.top;
	}
	POINT1 = cv::Point(XaxisX1, YaxisY1);
	POINT2 = cv::Point(XaxisX1 +9, YaxisY1 +9);
	radius = 9;
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

std::vector<UserPreferences> Pref::readFromCSV(const std::string& filename) {
	try{
		std::vector<UserPreferences> preferences;
		UserPreferences userPrefs;
		if (!loadCSV(filename, userPrefs)) {
			std::cerr << "No preferences found or error reading preferences. Creating with default values." << std::endl;
			getUserInput("Pillar height", userPrefs.height);
			getUserInput("threshold value for laser spot contrast", userPrefs.threshold);
			getUserInput("time of deposition", userPrefs.time);
			getUserInput("spot from left", userPrefs.left);
			getUserInput("spot from top", userPrefs.top);
			saveCSV(filename, userPrefs);
		}
		else {
			loadCSV(filename, userPrefs);
		}
		std::ifstream inFile(filename);
		if (!inFile.is_open()) {
			std::cerr << "Error opening file for reading." << std::endl;
			getUserInput("Pillar Height", userPrefs.height);
			getUserInput("threshold value for laser spot contrast", userPrefs.threshold);
			getUserInput("time of deposition", userPrefs.time);
			getUserInput("spot from left", userPrefs.left);
			getUserInput("spot from top", userPrefs.top);
			saveCSV(filename, userPrefs);
			return preferences;
		}
		std::string line;
		while (std::getline(inFile, line)) {
			std::istringstream iss(line);
			UserPreferences userPrefs;
			std::string volt, th, timed,left,top;
			if (std::getline(iss, volt, ',') &&
				std::getline(iss, th, ',') &&
				std::getline(iss, timed, ',') &&
				std::getline(iss, left, ',') &&
				std::getline(iss, top, ',')) {
				// Convert ageStr to an integer
				try {
					userPrefs.height = std::stod(volt);
					userPrefs.threshold = std::stod(th);
					userPrefs.time = std::stoi(timed); 
					userPrefs.left = std::stod(left);
					userPrefs.top = std::stoi(top);
				}
				catch (const std::invalid_argument& e) {
					std::cerr << "Error converting age to an integer: " << e.what() << std::endl;
					continue;
				}
				preferences.push_back(userPrefs);
			}
		}
		inFile.close();
		return preferences;
		}
	catch (int a) {
	
	};
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
		<< userPrefs.top;
	outFile.close();
}

bool Pref::loadCSV(const std::string& filename, UserPreferences& userPrefs) {
	std::ifstream inFile(filename);
	if (!inFile.is_open()) {
		return false; // File doesn't exist or cannot be opened; use default values
	}

	std::string line;
	std::getline(inFile, line);

	// Split the line into values using commas
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

	inFile.close();
	return true;
}

void Pref::app(const std::string& filename) {
	UserPreferences userPrefs;
	// Load preferences from a file or create it with default values
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
		case 'v':
			getUserInput("spot from left", userPrefs.left);
			break;
		case 'b':
			getUserInput("spot from top", userPrefs.top);
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

	system("cls");

	std::cout << "\t\t" << std::string(40, '_') << std::endl;
	std::cout << "\t\t" << "|" << std::string(38, ' ') << "\\" << std::endl;
	std::cout << "\t\t" << "|  Pillar Height(μm): " << userPrefs.height << std::endl;
	std::cout << "\t\t" << "|  Contrast:          " << userPrefs.threshold << std::endl;
	std::cout << "\t\t" << "|  Time for velocity: " << userPrefs.time << std::endl;
	std::cout << "\t\t" << "|  left:              " << userPrefs.left << std::endl;
	std::cout << "\t\t" << "|  top:               " << userPrefs.top << std::endl;
	std::cout << "\t\t" << "|" << std::string(38, ' ') << "/" << std::endl;
	std::cout << "\t\t" << std::string(40, '-') << std::endl;

}
void Pref::helpscreen() {
	string str[] = {
		" ",
		"z for height of pillar",
		"x for brightness threshold",
		"c for Time",
		"v for x coordinate of first point of select area",
		"b for y coordinate of first point of select area",
		" ",
		"q for BACK.<--|",
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
		"m to start deposition.",
		"b to start camera and deposition.",
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
	sis << stri ;
	return sis.str();
}
