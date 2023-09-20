#ifndef PREFERENCE_H
#define PREFERENCE_H

#include <string>
#include <vector>
#include <conio.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <nidaqmx.h>
#include <sstream>
#include <fstream>

#define PREF_FILE "C:/Users/nares/Desktop/allout/preference.csv"

using namespace std;

extern cv::Point POINT1;
extern cv::Point POINT2;
extern int radius;
extern int XaxisX1;
extern int YaxisY1;
extern double VOLTAGE;
extern double BRIGHTNESS;
extern int TTIME;
extern cv::Scalar red;
extern cv::Scalar white;
extern cv::Scalar black;
extern std::string commonPath;
//std::string commonPath = "C:/Users/iwata/OneDrive/Desktop/allout/";
extern char* dev0;
extern char* dev1;
extern int CAMERA;

struct UserPreferences {
	std::string name;
	double left;
	double top;
	double threshold;
	double height;
	int time;
	std::string favoriteColor;
	double camera;
};

class Pref{
public:
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
};

#endif // PREFERENCE_H
