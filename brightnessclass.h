#include "preference.h"

class BrightnessClass {
public:
	cv::Mat frame, grayFrame;
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

	double differencesOf() {
		int height = grayFrame.rows;
		int width = grayFrame.cols;
		static cv::Mat f1, f2, f3, f4, f5, f6, f7, f8, f9, f10,
			f11, f12, f13, f14, f15, f16, f17, f18, f19, f20,
			f21, f22, f23, f24, f25, f26, f27, f28, f29, f30;
		if (grayFrame.empty()) {
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
		else if (f4.empty()) {
			f4 = f3.clone();
			f3 = f2.clone();
			f2 = f1.clone();
			f1 = grayFrame.clone();
		}
		else if (f5.empty()) {
			f5 = f4.clone();
			f4 = f3.clone();
			f3 = f2.clone();
			f2 = f1.clone();
			f1 = grayFrame.clone();
		}
		else if (f6.empty()) {
			f6 = f5.clone();
			f5 = f4.clone();
			f4 = f3.clone();
			f3 = f2.clone();
			f2 = f1.clone();
			f1 = grayFrame.clone();
		}
		else if (f7.empty()) {
			f7 = f6.clone();
			f6 = f5.clone();
			f5 = f4.clone();
			f4 = f3.clone();
			f3 = f2.clone();
			f2 = f1.clone();
			f1 = grayFrame.clone();
		}
		else if (f8.empty()) {
			f8 = f7.clone();
			f7 = f6.clone();
			f6 = f5.clone();
			f5 = f4.clone();
			f4 = f3.clone();
			f3 = f2.clone();
			f2 = f1.clone();
			f1 = grayFrame.clone();
		}
		else if (f9.empty()) {
			f9 = f8.clone();
			f8 = f7.clone();
			f7 = f6.clone();
			f6 = f5.clone();
			f5 = f4.clone();
			f4 = f3.clone();
			f3 = f2.clone();
			f2 = f1.clone();
			f1 = grayFrame.clone();
		}
		else if (f10.empty()) {
			f10 = f9.clone();
			f9 = f8.clone();
			f8 = f7.clone();
			f7 = f6.clone();
			f6 = f5.clone();
			f5 = f4.clone();
			f4 = f3.clone();
			f3 = f2.clone();
			f2 = f1.clone();
			f1 = grayFrame.clone();
		}
		else if (f11.empty()) {
			f11 = f10.clone();
			f10 = f9.clone();
			f9 = f8.clone();
			f8 = f7.clone();
			f7 = f6.clone();
			f6 = f5.clone();
			f5 = f4.clone();
			f4 = f3.clone();
			f3 = f2.clone();
			f2 = f1.clone();
			f1 = grayFrame.clone();
		}
		else if (f12.empty()) {
			f12 = f11.clone();
			f11 = f10.clone();
			f10 = f9.clone();
			f9 = f8.clone();
			f8 = f7.clone();
			f7 = f6.clone();
			f6 = f5.clone();
			f5 = f4.clone();
			f4 = f3.clone();
			f3 = f2.clone();
			f2 = f1.clone();
			f1 = grayFrame.clone();
		}
		else if (f13.empty()) {
			f13 = f12.clone();
			f12 = f11.clone();
			f11 = f10.clone();
			f10 = f9.clone();
			f9 = f8.clone();
			f8 = f7.clone();
			f7 = f6.clone();
			f6 = f5.clone();
			f5 = f4.clone();
			f4 = f3.clone();
			f3 = f2.clone();
			f2 = f1.clone();
			f1 = grayFrame.clone();
		}
		else if (f14.empty()) {
			f14 = f13.clone();
			f13 = f12.clone();
			f12 = f11.clone();
			f11 = f10.clone();
			f10 = f9.clone();
			f9 = f8.clone();
			f8 = f7.clone();
			f7 = f6.clone();
			f6 = f5.clone();
			f5 = f4.clone();
			f4 = f3.clone();
			f3 = f2.clone();
			f2 = f1.clone();
			f1 = grayFrame.clone();
		}
		else if (f15.empty()) {
			f15 = f14.clone();
			f14 = f13.clone();
			f13 = f12.clone();
			f12 = f11.clone();
			f11 = f10.clone();
			f10 = f9.clone();
			f9 = f8.clone();
			f8 = f7.clone();
			f7 = f6.clone();
			f6 = f5.clone();
			f5 = f4.clone();
			f4 = f3.clone();
			f3 = f2.clone();
			f2 = f1.clone();
			f1 = grayFrame.clone();
		}
		else if (f16.empty()) {
			f16 = f15.clone();
			f15 = f14.clone();
			f14 = f13.clone();
			f13 = f12.clone();
			f12 = f11.clone();
			f11 = f10.clone();
			f10 = f9.clone();
			f9 = f8.clone();
			f8 = f7.clone();
			f7 = f6.clone();
			f6 = f5.clone();
			f5 = f4.clone();
			f4 = f3.clone();
			f3 = f2.clone();
			f2 = f1.clone();
			f1 = grayFrame.clone();
		}
		else if (f17.empty()) {
			f17 = f16.clone();
			f16 = f15.clone();
			f15 = f14.clone();
			f14 = f13.clone();
			f13 = f12.clone();
			f12 = f11.clone();
			f11 = f10.clone();
			f10 = f9.clone();
			f9 = f8.clone();
			f8 = f7.clone();
			f7 = f6.clone();
			f6 = f5.clone();
			f5 = f4.clone();
			f4 = f3.clone();
			f3 = f2.clone();
			f2 = f1.clone();
			f1 = grayFrame.clone();
		}
		else if (f18.empty()) {
			f18 = f17.clone();
			f17 = f16.clone();
			f16 = f15.clone();
			f15 = f14.clone();
			f14 = f13.clone();
			f13 = f12.clone();
			f12 = f11.clone();
			f11 = f10.clone();
			f10 = f9.clone();
			f9 = f8.clone();
			f8 = f7.clone();
			f7 = f6.clone();
			f6 = f5.clone();
			f5 = f4.clone();
			f4 = f3.clone();
			f3 = f2.clone();
			f2 = f1.clone();
			f1 = grayFrame.clone();
		}
		else if (f19.empty()) {
			f19 = f18.clone();
			f18 = f17.clone();
			f17 = f16.clone();
			f16 = f15.clone();
			f15 = f14.clone();
			f14 = f13.clone();
			f13 = f12.clone();
			f12 = f11.clone();
			f11 = f10.clone();
			f10 = f9.clone();
			f9 = f8.clone();
			f8 = f7.clone();
			f7 = f6.clone();
			f6 = f5.clone();
			f5 = f4.clone();
			f4 = f3.clone();
			f3 = f2.clone();
			f2 = f1.clone();
			f1 = grayFrame.clone();
		}
		else if (f20.empty()) {
			f20 = f19.clone();
			f19 = f18.clone();
			f18 = f17.clone();
			f17 = f16.clone();
			f16 = f15.clone();
			f15 = f14.clone();
			f14 = f13.clone();
			f13 = f12.clone();
			f12 = f11.clone();
			f11 = f10.clone();
			f10 = f9.clone();
			f9 = f8.clone();
			f8 = f7.clone();
			f7 = f6.clone();
			f6 = f5.clone();
			f5 = f4.clone();
			f4 = f3.clone();
			f3 = f2.clone();
			f2 = f1.clone();
			f1 = grayFrame.clone();
		}
		else if (f21.empty()) {
			f21 = f20.clone();
			f20 = f19.clone();
			f19 = f18.clone();
			f18 = f17.clone();
			f17 = f16.clone();
			f16 = f15.clone();
			f15 = f14.clone();
			f14 = f13.clone();
			f13 = f12.clone();
			f12 = f11.clone();
			f11 = f10.clone();
			f10 = f9.clone();
			f9 = f8.clone();
			f8 = f7.clone();
			f7 = f6.clone();
			f6 = f5.clone();
			f5 = f4.clone();
			f4 = f3.clone();
			f3 = f2.clone();
			f2 = f1.clone();
			f1 = grayFrame.clone();
		}
		else if (f22.empty()) {
			f22 = f21.clone();
			f21 = f20.clone();
			f20 = f19.clone();
			f19 = f18.clone();
			f18 = f17.clone();
			f17 = f16.clone();
			f16 = f15.clone();
			f15 = f14.clone();
			f14 = f13.clone();
			f13 = f12.clone();
			f12 = f11.clone();
			f11 = f10.clone();
			f10 = f9.clone();
			f9 = f8.clone();
			f8 = f7.clone();
			f7 = f6.clone();
			f6 = f5.clone();
			f5 = f4.clone();
			f4 = f3.clone();
			f3 = f2.clone();
			f2 = f1.clone();
			f1 = grayFrame.clone();
		}
		else if (f23.empty()) {
			f23 = f22.clone();
			f22 = f21.clone();
			f21 = f20.clone();
			f20 = f19.clone();
			f19 = f18.clone();
			f18 = f17.clone();
			f17 = f16.clone();
			f16 = f15.clone();
			f15 = f14.clone();
			f14 = f13.clone();
			f13 = f12.clone();
			f12 = f11.clone();
			f11 = f10.clone();
			f10 = f9.clone();
			f9 = f8.clone();
			f8 = f7.clone();
			f7 = f6.clone();
			f6 = f5.clone();
			f5 = f4.clone();
			f4 = f3.clone();
			f3 = f2.clone();
			f2 = f1.clone();
			f1 = grayFrame.clone();
		}
		else if (f24.empty()) {
			f24 = f23.clone();
			f23 = f22.clone();
			f22 = f21.clone();
			f21 = f20.clone();
			f20 = f19.clone();
			f19 = f18.clone();
			f18 = f17.clone();
			f17 = f16.clone();
			f16 = f15.clone();
			f15 = f14.clone();
			f14 = f13.clone();
			f13 = f12.clone();
			f12 = f11.clone();
			f11 = f10.clone();
			f10 = f9.clone();
			f9 = f8.clone();
			f8 = f7.clone();
			f7 = f6.clone();
			f6 = f5.clone();
			f5 = f4.clone();
			f4 = f3.clone();
			f3 = f2.clone();
			f2 = f1.clone();
			f1 = grayFrame.clone();
		}
		else if (f25.empty()) {
			f25 = f24.clone();
			f24 = f23.clone();
			f23 = f22.clone();
			f22 = f21.clone();
			f21 = f20.clone();
			f20 = f19.clone();
			f19 = f18.clone();
			f18 = f17.clone();
			f17 = f16.clone();
			f16 = f15.clone();
			f15 = f14.clone();
			f14 = f13.clone();
			f13 = f12.clone();
			f12 = f11.clone();
			f11 = f10.clone();
			f10 = f9.clone();
			f9 = f8.clone();
			f8 = f7.clone();
			f7 = f6.clone();
			f6 = f5.clone();
			f5 = f4.clone();
			f4 = f3.clone();
			f3 = f2.clone();
			f2 = f1.clone();
			f1 = grayFrame.clone();
		}
		else if (f26.empty()) {
			f26 = f25.clone();
			f25 = f24.clone();
			f24 = f23.clone();
			f23 = f22.clone();
			f22 = f21.clone();
			f21 = f20.clone();
			f20 = f19.clone();
			f19 = f18.clone();
			f18 = f17.clone();
			f17 = f16.clone();
			f16 = f15.clone();
			f15 = f14.clone();
			f14 = f13.clone();
			f13 = f12.clone();
			f12 = f11.clone();
			f11 = f10.clone();
			f10 = f9.clone();
			f9 = f8.clone();
			f8 = f7.clone();
			f7 = f6.clone();
			f6 = f5.clone();
			f5 = f4.clone();
			f4 = f3.clone();
			f3 = f2.clone();
			f2 = f1.clone();
			f1 = grayFrame.clone();
		}
		else if (f27.empty()) {
			f27 = f26.clone();
			f26 = f25.clone();
			f25 = f24.clone();
			f24 = f23.clone();
			f23 = f22.clone();
			f22 = f21.clone();
			f21 = f20.clone();
			f20 = f19.clone();
			f19 = f18.clone();
			f18 = f17.clone();
			f17 = f16.clone();
			f16 = f15.clone();
			f15 = f14.clone();
			f14 = f13.clone();
			f13 = f12.clone();
			f12 = f11.clone();
			f11 = f10.clone();
			f10 = f9.clone();
			f9 = f8.clone();
			f8 = f7.clone();
			f7 = f6.clone();
			f6 = f5.clone();
			f5 = f4.clone();
			f4 = f3.clone();
			f3 = f2.clone();
			f2 = f1.clone();
			f1 = grayFrame.clone();
		}
		else if (f28.empty()) {
			f28 = f27.clone();
			f27 = f26.clone();
			f26 = f25.clone();
			f25 = f24.clone();
			f24 = f23.clone();
			f23 = f22.clone();
			f22 = f21.clone();
			f21 = f20.clone();
			f20 = f19.clone();
			f19 = f18.clone();
			f18 = f17.clone();
			f17 = f16.clone();
			f16 = f15.clone();
			f15 = f14.clone();
			f14 = f13.clone();
			f13 = f12.clone();
			f12 = f11.clone();
			f11 = f10.clone();
			f10 = f9.clone();
			f9 = f8.clone();
			f8 = f7.clone();
			f7 = f6.clone();
			f6 = f5.clone();
			f5 = f4.clone();
			f4 = f3.clone();
			f3 = f2.clone();
			f2 = f1.clone();
			f1 = grayFrame.clone();
		}
		else if (f29.empty()) {
			f29 = f28.clone();
			f28 = f27.clone();
			f27 = f26.clone();
			f26 = f25.clone();
			f25 = f24.clone();
			f24 = f23.clone();
			f23 = f22.clone();
			f22 = f21.clone();
			f21 = f20.clone();
			f20 = f19.clone();
			f19 = f18.clone();
			f18 = f17.clone();
			f17 = f16.clone();
			f16 = f15.clone();
			f15 = f14.clone();
			f14 = f13.clone();
			f13 = f12.clone();
			f12 = f11.clone();
			f11 = f10.clone();
			f10 = f9.clone();
			f9 = f8.clone();
			f8 = f7.clone();
			f7 = f6.clone();
			f6 = f5.clone();
			f5 = f4.clone();
			f4 = f3.clone();
			f3 = f2.clone();
			f2 = f1.clone();
			f1 = grayFrame.clone();
		}
		else if (f30.empty()) {
			f30 = f29.clone();
			f29 = f28.clone();
			f28 = f27.clone();
			f27 = f26.clone();
			f26 = f25.clone();
			f25 = f24.clone();
			f24 = f23.clone();
			f23 = f22.clone();
			f22 = f21.clone();
			f21 = f20.clone();
			f20 = f19.clone();
			f19 = f18.clone();
			f18 = f17.clone();
			f17 = f16.clone();
			f16 = f15.clone();
			f15 = f14.clone();
			f14 = f13.clone();
			f13 = f12.clone();
			f12 = f11.clone();
			f11 = f10.clone();
			f10 = f9.clone();
			f9 = f8.clone();
			f8 = f7.clone();
			f7 = f6.clone();
			f6 = f5.clone();
			f5 = f4.clone();
			f4 = f3.clone();
			f3 = f2.clone();
			f2 = f1.clone();
			f1 = grayFrame.clone();
		}
		else {
			f30 = f29.clone();
			f29 = f28.clone();
			f28 = f27.clone();
			f27 = f26.clone();
			f26 = f25.clone();
			f25 = f24.clone();
			f24 = f23.clone();
			f23 = f22.clone();
			f22 = f21.clone();
			f21 = f20.clone();
			f20 = f19.clone();
			f19 = f18.clone();
			f18 = f17.clone();
			f17 = f16.clone();
			f16 = f15.clone();
			f15 = f14.clone();
			f14 = f13.clone();
			f13 = f12.clone();
			f12 = f11.clone();
			f11 = f10.clone();
			f10 = f9.clone();
			f9 = f8.clone();
			f8 = f7.clone();
			f7 = f6.clone();
			f6 = f5.clone();
			f5 = f4.clone();
			f4 = f3.clone();
			f3 = f2.clone();
			f2 = f1.clone();
			f1 = grayFrame.clone();
		}
		double Pij = 0.0;
		if (!f1.empty() && !f30.empty()) {
			for (int y = 0; y < height; ++y) {
				uchar* row_ptr1 = f1.ptr<uchar>(y);
				uchar* row_ptr2 = f2.ptr<uchar>(y);
				uchar* row_ptr3 = f3.ptr<uchar>(y);
				uchar* row_ptr4 = f4.ptr<uchar>(y);
				uchar* row_ptr5 = f5.ptr<uchar>(y);
				uchar* row_ptr6 = f6.ptr<uchar>(y);
				uchar* row_ptr7 = f7.ptr<uchar>(y);
				uchar* row_ptr8 = f8.ptr<uchar>(y);
				uchar* row_ptr9 = f9.ptr<uchar>(y);
				uchar* row_ptr10 = f10.ptr<uchar>(y);
				uchar* row_ptr11 = f11.ptr<uchar>(y);
				uchar* row_ptr12 = f12.ptr<uchar>(y);
				uchar* row_ptr13 = f13.ptr<uchar>(y);
				uchar* row_ptr14 = f14.ptr<uchar>(y);
				uchar* row_ptr15 = f15.ptr<uchar>(y);
				uchar* row_ptr16 = f16.ptr<uchar>(y);
				uchar* row_ptr17 = f17.ptr<uchar>(y);
				uchar* row_ptr18 = f18.ptr<uchar>(y);
				uchar* row_ptr19 = f19.ptr<uchar>(y);
				uchar* row_ptr20 = f20.ptr<uchar>(y);
				uchar* row_ptr21 = f21.ptr<uchar>(y);
				uchar* row_ptr22 = f22.ptr<uchar>(y);
				uchar* row_ptr23 = f23.ptr<uchar>(y);
				uchar* row_ptr24 = f24.ptr<uchar>(y);
				uchar* row_ptr25 = f25.ptr<uchar>(y);
				uchar* row_ptr26 = f26.ptr<uchar>(y);
				uchar* row_ptr27 = f27.ptr<uchar>(y);
				uchar* row_ptr28 = f28.ptr<uchar>(y);
				uchar* row_ptr29 = f29.ptr<uchar>(y);
				uchar* row_ptr30 = f30.ptr<uchar>(y);
				for (int x = 0; x < width; ++x) {
					int cf1 = row_ptr1[x];
					int cf2 = row_ptr2[x];
					int cf3 = row_ptr3[x];
					int cf4 = row_ptr4[x];
					int cf5 = row_ptr5[x];
					int cf6 = row_ptr6[x];
					int cf7 = row_ptr7[x];
					int cf8 = row_ptr8[x];
					int cf9 = row_ptr9[x];
					int cf10 = row_ptr10[x];
					int cf11 = row_ptr11[x];
					int cf12 = row_ptr12[x];
					int cf13 = row_ptr13[x];
					int cf14 = row_ptr14[x];
					int cf15 = row_ptr15[x];
					int cf16 = row_ptr16[x];
					int cf17 = row_ptr17[x];
					int cf18 = row_ptr18[x];
					int cf19 = row_ptr19[x];
					int cf20 = row_ptr20[x];
					int cf21 = row_ptr21[x];
					int cf22 = row_ptr22[x];
					int cf23 = row_ptr23[x];
					int cf24 = row_ptr24[x];
					int cf25 = row_ptr25[x];
					int cf26 = row_ptr26[x];
					int cf27 = row_ptr27[x];
					int cf28 = row_ptr28[x];
					int cf29 = row_ptr29[x];
					int cf30 = row_ptr30[x];

					int pd1 = abs(cf1 - cf2);
					int pd2 = abs(cf1 - cf3);
					int pd3 = abs(cf1 - cf4);
					int pd4 = abs(cf1 - cf5);
					int pd5 = abs(cf1 - cf6);
					int pd6 = abs(cf1 - cf7);
					int pd7 = abs(cf1 - cf8);
					int pd8 = abs(cf1 - cf9);
					int pd9 = abs(cf1 - cf10);
					int pd10 = abs(cf1 - cf11);
					int pd11 = abs(cf1 - cf12);
					int pd12 = abs(cf1 - cf13);
					int pd13 = abs(cf1 - cf14);
					int pd14 = abs(cf1 - cf15);
					int pd15 = abs(cf1 - cf16);
					int pd16 = abs(cf1 - cf17);
					int pd17 = abs(cf1 - cf18);
					int pd18 = abs(cf1 - cf19);
					int pd19 = abs(cf1 - cf20);
					int pd20 = abs(cf1 - cf21);
					int pd21 = abs(cf1 - cf22);
					int pd22 = abs(cf1 - cf23);
					int pd23 = abs(cf1 - cf24);
					int pd24 = abs(cf1 - cf25);
					int pd25 = abs(cf1 - cf26);
					int pd26 = abs(cf1 - cf27);
					int pd27 = abs(cf1 - cf28);
					int pd28 = abs(cf1 - cf29);
					int pd29 = abs(cf1 - cf30);
					Pij = pd26;
				}
			}
		}
		else {
			std::cerr << "Error: Frames f1 or f3 are empty." << std::endl;
			return 0.0;
		}
		return Pij;
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
	
	double avgotp() {
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
};


