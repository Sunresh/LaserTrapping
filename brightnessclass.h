#include "preference.h"

class BrightnessClass {
public:
	cv::Mat frame, grayFrame;;
	std::deque<double> framedata;
	double upperlimit,meanCV,variance;
	BrightnessClass(cv::Mat& frame);
	~BrightnessClass();
	double BrightnessClass::avg();
	double BrightnessClass::stdev();
	void BrightnessClass::setUpperlimit(double uplimit);
	double BrightnessClass::getUpperlimit();
	int width, height;
};