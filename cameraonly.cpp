#include "cameraonly.h"
#include <conio.h>
#include "preference.h"



CameraOnly::CameraOnly() {
	cam.open(1);
}

CameraOnly::~CameraOnly() {
	cam.release();
	cv::destroyAllWindows();
}
void CameraOnly::application() {
	if (!cam.isOpened()) {
		return;
	}
	cv::Mat dframe, grayColorRect;
	while (true) {
		cam >> dframe;
		if (dframe.empty()) {
			break;
		}
		cv::rectangle(dframe, pt1, pt2, red, 1);
		cv::Rect roiRect(x1 + 1, y1 + 1, roiw - 2, roih - 2); // (x, y, width, height)

		cv::imshow("Status of deposition", dframe);
		cv::moveWindow("Status of deposition", 0, 0);

		char key = cv::waitKey(1);
		if (key == 'q' || key == ' ') {
			cam.release();
			cv::destroyAllWindows();
			break;
		}
	}
}
