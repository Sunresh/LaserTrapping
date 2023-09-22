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

#define PREF_FILE commonPath+"preference.csv"
extern std::string LAST_VOLT_FILE;
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
private:
	int height;
	double threshold;
	int time;

};

#endif // PREFERENCE_H
