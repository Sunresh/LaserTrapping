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
		int height = grayFrame.rows;
		int width = grayFrame.cols;

		static cv::Mat f1, f2, f3;

		double th1 = 0.12 * 255;
		double th2 = 0.25 * 255;
		double th3 = 0.5 * 255;
		double th4 = 0.75 * 255;

		if (grayFrame.empty()) {
			// Handling if the input frame is empty
			std::cerr << "Error: Empty input frame." << std::endl;
			return 0.0;
		}

		if (f1.empty()) {
			f1 = grayFrame.clone();
		}
		else if (f2.empty()) {
			f2 = f1.clone();
			f1 = grayFrame.clone();
		}
		else if (f3.empty()) {
			f3 = f2.clone();
			f2 = f1.clone();
			f1 = grayFrame.clone();
		}
		else {
			// Randomize the assignment of frames for comparison
			int randNum = rand() % 3;
			if (randNum == 0) {
				f3 = f2.clone();
				f2 = f1.clone();
				f1 = grayFrame.clone();
			}
			else if (randNum == 1) {
				f1 = f3.clone();
				f3 = f2.clone();
				f2 = grayFrame.clone();
			}
			else {
				f2 = f1.clone();
				f1 = f3.clone();
				f3 = grayFrame.clone();
			}
		}

		double sumPij = 0.0;
		double previousPixelDiff = 0.0;

		if (!f1.empty() && !f3.empty()) {
			for (int y = 0; y < height; ++y) {
				for (int x = 0; x < width; ++x) {
					// Calculate the brightness difference based on the thresholds
					int cf1 = f1.at<uchar>(y, x);
					int cf2 = f2.at<uchar>(y, x);
					int cf3 = f3.at<uchar>(y, x);
					//int pixelDiff = abs(3 * f1.at<uchar>(y, x) - f2.at<uchar>(y, x) - f3.at<uchar>(y, x));
					int pixelDiff = abs(3*cf1-cf2-cf3);

					if (pixelDiff > th3 && pixelDiff <= th4) {
						double diffChange = previousPixelDiff - pixelDiff;
						// Apply previous pixelDiff comparison logic
						if (th1 < diffChange && diffChange < th2) {
							if (previousPixelDiff < pixelDiff) {
								sumPij += pixelDiff * 4; // Multiply by 8 if current is greater than previous
							}
							else {
								sumPij += pixelDiff * 0.0001; // Multiply by 0.001 if current is smaller than previous
							}
						}
						else if (th2 < diffChange && diffChange < th3) {
							if (previousPixelDiff < pixelDiff) {
								sumPij += pixelDiff * 3; // Multiply by 8 if current is greater than previous
							}
							else {
								sumPij += pixelDiff * 0.0001; // Multiply by 0.001 if current is smaller than previous
							}
						}
						else if (diffChange > th3) {
							if (previousPixelDiff < pixelDiff) {
								sumPij += pixelDiff * 2; // Multiply by 8 if current is greater than previous
							}
							else {
								sumPij += pixelDiff * 0.01; // Multiply by 0.001 if current is smaller than previous
							}
						}
						else {
							sumPij += pixelDiff * 8; // High weight for mid-high range differences
						}
					}
					else if (pixelDiff > th2 && pixelDiff <= th3) {
						sumPij += pixelDiff * 0.0001; // Low weight for mid-low range differences
					}
					else if (pixelDiff > th1 && pixelDiff <= th2) {
						sumPij += pixelDiff * 14; // Moderate weight for low-mid range differences
					}
					else {
						sumPij += pixelDiff * 7; // Normal contribution from differences within thresholds
					}
					previousPixelDiff = pixelDiff;
				}
			}
		}
		else {
			std::cerr << "Error: Frames f1 or f3 are empty." << std::endl;
			return 0.0;
		}
	
		if (sumPij > (255 * width * height)*0.9) {
			static int frameCount = 0;
			if (frameCount % 2 == 0) {
				int randomValue = rand() % 3;
				sumPij = (255 * width * height)*0.9* randomValue/2;
			}
			else if(frameCount % 2 == 1){
				int randomValue = rand() % 3;
				sumPij = 0.1 * randomValue / 2;
			}
			frameCount++;
		}
		else if(sumPij < 0) {
			static int frameCount = 0;
			if (frameCount % 2 == 0) {
				int randomValue = rand() % 3;
				sumPij = 0.1 * randomValue / 2;
			}
			else if (frameCount % 2 == 1) {
				int randomValue = rand() % 3;
				sumPij = 1 * randomValue / 2;
			}
			frameCount++;
		}
		
		double contrast = sumPij / (255 * width * height);
		return contrast;
	}

	double avgot() {
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