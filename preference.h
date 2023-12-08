#ifndef PREFERENCE_H
#define PREFERENCE_H

#include <NIDAQmx.h>
#include <string>
#include <vector>
#include <conio.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <nidaqmx.h>
#include <sstream>
#include <fstream>
#include <Windows.h>
#include <ShlObj.h>
#include <cfloat>

using namespace std;

struct UserPreferences {
	std::string name, favoriteColor;
	double left, top, sdupperpoint, sdlowerpoint, height, wofs, time, camera,hofs;
};
class SchmittTrigger {
public:
	SchmittTrigger(double upperThreshold, double lowerThreshold)
		: upperThreshold_(upperThreshold), lowerThreshold_(lowerThreshold), output_(false) {
	}
	bool processInput(double inputValue) {
		if (inputValue >= upperThreshold_) {
			output_ = true;
		}
		else if (inputValue <= lowerThreshold_) {
			output_ = false;
		}
		return output_;
	}
	bool getOutput() const {
		return output_;
	}

private:
	double upperThreshold_;
	double lowerThreshold_;
	bool output_;
};

class Memory {
private:
	std::string commonPath;

public:
	void storeValue(double val) {
		// Store the value in the file
		std::ofstream file(commonPath+"stored_value.txt");
		if (file.is_open()) {
			file << val;
			file.close();
		}
	}
	double getValue() const {
		double storedValue = 0;
		// Retrieve the stored value from the file
		std::ifstream file(commonPath+"stored_value.txt");
		if (file.is_open()) {
			file >> storedValue;
			file.close();
		}
		return storedValue;
	}
};


class Pref{
private:
	std::string DesktopFolder;
public:
	static const int SCREEN_HEIGHT = 600;
	static const int SCREEN_WIDTH = 600;
	UserPreferences prefs;
	double gethofs() {
		LoadPreferences();
		return prefs.hofs;
	}

	Pref::Pref() : height(), threshold(), time() {
		PWSTR path;
		if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Desktop, 0, NULL, &path))) {
			DesktopFolder = std::string(path, path + wcslen(path));
			CoTaskMemFree(path);
		}
		else {
			std::wcerr << L"Failed to get the desktop path" << std::endl;
		}
		LoadPreferences();
	}
	cv::Scalar uBGR(int i,int j,int k) {
		return cv::Scalar(i,j,k);
	}
	string getCommonPath() {
		return DesktopFolder + "/allout/";
	}
	string getprefPath() {
		return DesktopFolder + "/allout/"+"preference.csv";
	}
	char* deva0() {
		return "Dev2/ao0";
	}
	char* deva1() {
		return "Dev2/ao1";
	}
	void Pref::LoadPreferences() {
		if (!loadCSV(getprefPath(), prefs)) {
			std::cerr << "No preferences found or error reading preferences. Creating with default values." << std::endl;
		}
		else {
			loadCSV(getprefPath(), prefs);
		}
	}
	double Pref::getCameraId() {
		LoadPreferences();
		return prefs.camera;
	}
	double Pref::getUpperTh() {
		LoadPreferences();
		return prefs.sdupperpoint;
	}
	double Pref::getLowerTh() {
		LoadPreferences();
		return prefs.sdlowerpoint;
	}
	double Pref::getDurationTime() {
		LoadPreferences();
		return prefs.time;
	}
	double Pref::getRadiusBox() {
		LoadPreferences();
		return prefs.wofs;
	}
	double Pref::getTop() {
		LoadPreferences();
		return prefs.top;
	}
	double Pref::getLeft() {
		LoadPreferences();
		return prefs.left;
	}
	double Pref::maxVolt() {
		LoadPreferences();
		return prefs.height / 6;
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
		getUserInput("threshold upper for laser spot", userPrefs.sdupperpoint);
		getUserInput("time of deposition", userPrefs.time);
		getUserInput("spot from left", userPrefs.left);
		getUserInput("spot from top", userPrefs.top);
		saveCSV(getprefPath(), userPrefs);
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
			<< userPrefs.sdupperpoint << ","
			<< userPrefs.time << ","
			<< userPrefs.left << ","
			<< userPrefs.top << ","
			<< userPrefs.camera << ","
			<< userPrefs.wofs << ","
			<< userPrefs.sdlowerpoint << ","
			<< userPrefs.hofs << endl;
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
		userPrefs.height = std::stod(value);
		getline(ss, value, ',');
		userPrefs.sdupperpoint = std::stod(value);
		getline(ss, value, ',');
		userPrefs.time = std::stod(value);
		getline(ss, value, ',');
		userPrefs.left = std::stod(value);
		getline(ss, value, ',');
		userPrefs.top = std::stod(value);
		getline(ss, value, ',');
		userPrefs.camera = std::stod(value);
		getline(ss, value, ',');
		userPrefs.wofs = std::stod(value);
		getline(ss, value, ',');
		userPrefs.sdlowerpoint = std::stod(value);
		getline(ss, value, ',');
		userPrefs.hofs = std::stod(value);

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
			key = _getch(); // Use getchar to read a character
			switch (key) {
			case 'z':
				getUserInput("Pillar Height", userPrefs.height);
				break;
			case 'x':
				getUserInput("threshold upper", userPrefs.sdupperpoint);
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
				getUserInput("spot width", userPrefs.wofs);
				break;
			case 'w':
				getDirectV("Webcam", userPrefs.camera, "0");
				break;
			case 'W':
				getDirectV("CCD cam", userPrefs.camera, "1");
				break;
			case 'p':
				getUserInput("Lower point", userPrefs.sdlowerpoint);
				break;
			case '1':
				getUserInput("height of spot", userPrefs.hofs);
				break;
			case 'q':
				break;
			default:
				continue;
			}
			saveCSV(filename, userPrefs);
			break;
		}
	}

	void Pref::startscreen() {
		UserPreferences userPrefs;
		if (!loadCSV(getprefPath(), userPrefs)) {
			std::cerr << "No preferences found or error reading preferences. Creating with default values." << std::endl;
		}
		else {
			loadCSV(getprefPath(), userPrefs);
		}
		//	system("cls");
		std::cout << "\t\t" << std::string(48, '_') << std::endl;
		std::cout << "\t\t" << "|" << std::string(46, ' ') << "|" << std::endl;
		std::cout << "\t\t" << "|  z. Pillar Height(micro-m): " << userPrefs.height << std::string(15, ' ') << "|" << std::endl;
		std::cout << "\t\t" << "|  x. SDUpperP:               " << userPrefs.sdupperpoint << std::string(16, ' ') << "|" << std::endl;
		std::cout << "\t\t" << "|  p. SDLOWp:                 " << userPrefs.sdlowerpoint << std::string(14, ' ') << "|" << std::endl;
		std::cout << "\t\t" << "|  c. Time for velocity:      " << userPrefs.time << std::string(16, ' ') << "|" << std::endl;
		std::cout << "\t\t" << "|  l. left:                   " << userPrefs.left << std::string(14, ' ') << "|" << std::endl;
		std::cout << "\t\t" << "|  t. top:                    " << userPrefs.top << std::string(14, ' ') << "|" << std::endl;
		std::cout << "\t\t" << "|  r. WOFS:                   " << userPrefs.wofs << std::string(14, ' ') << "|" << std::endl;
		std::cout << "\t\t" << "|  1. HOFS:                   " << userPrefs.hofs << std::string(14, ' ') << "|" << std::endl;
		if (userPrefs.camera == 0) {
			std::cout << "\t\t" << "|  w. Camera:                 " << "Webcam(W)" << std::string(8, ' ') << "|" << std::endl;
		}
		if (userPrefs.camera == 1) {
			std::cout << "\t\t" << "|  W. Camera:                 " << "CCD Camera(w)" << std::string(7, ' ') << "|" << std::endl;
		}
		std::cout << "\t\t" << "|" << std::string(46, ' ') << "|" << std::endl;
		std::cout << "\t\t" << "|  q. Back:                   " << std::string(17, ' ') << "|" << std::endl;
		std::cout << "\t\t" << "|  e. Edit:                   " << std::string(17, ' ') << "|" << std::endl;
		std::cout << "\t\t" << "|  c&k. camera and deposition:" << std::string(17, ' ') << "|" << std::endl;
		std::cout << "\t\t" << std::string(48, '-') << std::endl;
	}

	void Pref::simpleCSVsave(std::string& filename, double value) {
		std::string fileout = getCommonPath() + filename;
		std::ofstream file(fileout);
		if (!file.is_open()) {
			std::cerr << "Failed to open the file." << std::endl;
			return;
		}
		file << value << "\n";
		file.close();
	}


private:
	double height,threshold, time;

};

#endif // PREFERENCE_H
