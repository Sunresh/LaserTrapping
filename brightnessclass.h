#include "preference.h"

class BrightnessClass {
public:
	cv::Mat frame, grayFrame;;
	double upperlimit,meanCV,variance;
	BrightnessClass(cv::Mat& frame);
	~BrightnessClass();
	double BrightnessClass::avg();
	double BrightnessClass::stdev();
	void BrightnessClass::setUpperlimit(double uplimit);
	double BrightnessClass::getUpperlimit();
	double BrightnessClass::sddif(std::deque<double>& graphValues);
	int width, height;
};