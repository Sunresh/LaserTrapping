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

#define PREF_FILE commonPath+"preference.csv"
extern std::string LAST_VOLT_FILE;
using namespace std;

extern cv::Point POINT1;
extern cv::Point POINT2;
extern cv::Scalar red, white, black, green;
extern std::string commonPath;
extern char* dev0;
extern char* dev1;

struct UserPreferences {
	std::string name, favoriteColor;
	double left, top, threshold, sdlowerpoint, height, radius, time, camera;
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
public:
	static const int SCREEN_HEIGHT = 600;
	static const int SCREEN_WIDTH = 600;
	Pref();
	bool isNumeric(const std::string& str);
	void getUserInput(const std::string& fieldName, int& field);
	void getUserInput(const std::string& fieldName, double& field);
	void getDirectV(const std::string& fieldName, double& field, const string& val);
	void saveCSV(const std::string& filename, const UserPreferences& userPrefs);
	bool loadCSV(const std::string& filename, UserPreferences& userPrefs);
	void app(const std::string& filename);
	void startscreen();
	void helpscreen();
	void Pref::menu();
	std::string double2string(const double& value, const string& stri);
	void Pref::allentry();
	void simpleCSVsave(std::string& filename,double value);
	std::string simpleCSVread(std::string& filename);
	void slowlyslowly(std::string& filename);
	void Pref::createDefaultFile(const std::string& filename);
	void Pref::setHeight(int newHeight);
	int getHeight() const;
	double Pref::maxVolt();
	void Pref::LoadPreferences();
	double Pref::getUpperTh();
	double Pref::getLowerTh();
	double Pref::getTop();
	double Pref::getLeft();
	double Pref::getDurationTime();
	double Pref::getRadiusBox(); 
	double Pref::getCameraId();

private:
	double height,threshold, time;

};

#endif // PREFERENCE_H
