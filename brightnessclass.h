#include "preference.h"

class BrightnessClass {
public:
	cv::Mat frame, grayFrame;;
	std::deque<double> framedata;
	double upperlimit,meanCV,variance;
	int width, height;

	BrightnessClass(cv::Mat& iframe) {
		frame = iframe;
		cv::cvtColor(iframe, grayFrame, cv::COLOR_BGR2GRAY);
		width = grayFrame.cols;
		height = grayFrame.rows;
		variance = 0.0;
		meanCV = cv::mean(grayFrame)[0];
	};
	~BrightnessClass() {};
	void setUpperlimit(double uplimit) {
		upperlimit = uplimit;
	}
	double BrightnessClass::getUpperlimit() {
		return upperlimit;
	}

	double avg() {
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

	double stdev() {
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
};