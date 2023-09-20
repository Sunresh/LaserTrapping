#include "preference.h"


class MainWindow {
public:
	Pref pr;
	MainWindow();
	~MainWindow();
	double durationInSeconds = 2;
	double pztvolt = 0;
	double thContrast = 0;
	void app();
};