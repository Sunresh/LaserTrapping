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
	double standardDeviation = std::sqrt(variance);
	return standardDeviation;
}

//double BrightnessClass::sddif(std::deque<double>& graphValues) {
//	double averagediff = 0;
//	int numPoints = graphValues.size();
//	double varr = 0;
//	for (int i = 10; i < numPoints; ++i) {
//		double mwn = cv::mean(graphValues[i-10])[0];//here i want to calculate mean fraom previous 10 data please help here
//		varr = std::pow(graphValues[i] - mwn, 2);
//		cout << mwn << endl;
//		//averagediff += std::abs(graphValues[i] - (graphValues[i - 1] + graphValues[i - 2] + graphValues[i - 3] + graphValues[i - 4]) / 4);
//	}
//	varr /= numPoints;
//	double standardDeviation = std::sqrt(varr);
//	return standardDeviation;
//}

double BrightnessClass::sddif(std::deque<double>& graphValues) {
	double averagediff = 0;
	int numPoints = graphValues.size();
	double variance = 0;
	for (int i = 10; i < numPoints; ++i) {
		double sum = 0;
		for (int j = i - 10; j < i; ++j) {
			sum += graphValues[j];
		}
		double meanOfPrev10 = sum / 10;
		variance += std::pow(graphValues[i] - meanOfPrev10, 2);
	}
	variance /= (numPoints-10);
	double standardDeviation = std::sqrt(variance);
	return standardDeviation;
}

