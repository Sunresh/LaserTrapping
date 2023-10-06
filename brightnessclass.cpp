#include "brightnessclass.h"

BrightnessClass::BrightnessClass(cv::Mat& iframe): frame(), upperlimit(), meanCV(){
	frame = iframe;
	cv::cvtColor(iframe, grayFrame, cv::COLOR_BGR2GRAY);
	width = grayFrame.cols;
	height = grayFrame.rows;
	variance = 0.0;
	meanCV = cv::mean(grayFrame)[0];

}
void BrightnessClass::setUpperlimit(double uplimit) {
	upperlimit = uplimit;
}
double BrightnessClass::getUpperlimit() {
	return upperlimit;
}
BrightnessClass::~BrightnessClass() {}

double BrightnessClass::avg() {
	setUpperlimit(1);
	double sumPij = 0.0;
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			sumPij += grayFrame.at<uchar>(y, x);
		}
	}
	double contrast = sumPij / (255 * width * height);
	return contrast;
}

double BrightnessClass::stdev() {
	setUpperlimit(100);
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			double pixelValue = static_cast<double>(grayFrame.at<uchar>(y, x));
			variance += std::pow(pixelValue - meanCV, 2);
		}
	}
	variance /= (width * height);
	double bright = std::sqrt(variance);
	return bright;
}
