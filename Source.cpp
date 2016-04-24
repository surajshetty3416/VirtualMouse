#include <sstream>
#include <string.h>
#include <iostream>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\opencv.hpp>
#include <Windows.h>

using namespace cv;
using namespace std;

int H_MIN = 0;
int H_MAX = 180;
int S_MIN = 0;
int S_MAX = 256;
int V_MIN = 0;
int V_MAX = 256;

int POINTER1_H_MIN = 0;
int POINTER1_H_MAX = 180;
int POINTER1_S_MIN = 0;
int POINTER1_S_MAX = 256;
int POINTER1_V_MIN = 0;
int POINTER1_V_MAX = 256;

int POINTER2_H_MIN = 0;
int POINTER2_H_MAX = 180;
int POINTER2_S_MIN = 0;
int POINTER2_S_MAX = 256;
int POINTER2_V_MIN = 0;
int POINTER2_V_MAX = 256;

int POINTER3_H_MIN = 0;
int POINTER3_H_MAX = 180;
int POINTER3_S_MIN = 0;
int POINTER3_S_MAX = 256;
int POINTER3_V_MIN = 0;
int POINTER3_V_MAX = 256;

int TRACK_STATUS = 0;
int FILTER_FOR = 0;
int Px1 = 0, Py1 = 0, Px2 = 0, Py2 = 0, Px3 = 0,Py3 = 0;

bool POINTER1_FOUND = FALSE;
bool POINTER2_FOUND = FALSE;
bool POINTER3_FOUND = FALSE;
bool LCLICK = FALSE;
bool RCLICK = FALSE;
bool calibrationMode;
bool mouseIsDragging;
bool mouseMove;
bool rectangleSelected;

cv::Point initialClickPoint, currentMousePoint; 
cv::Rect rectangleROI; 

vector<int> H_ROI, S_ROI, V_ROI;

const int USER_SCREEN_WIDTH = GetSystemMetrics(SM_CXSCREEN);
const int USER_SCREEN_HEIGHT = GetSystemMetrics(SM_CYSCREEN);

const int FRAME_WIDTH = 640;
const int FRAME_HEIGHT = 480;

const int MAX_NUM_OBJECTS = 25;

const int MIN_OBJECT_AREA = 10 * 10;
const int MAX_OBJECT_AREA = FRAME_HEIGHT*FRAME_WIDTH / 4;

const string windowName = "Original Image";
const string trackbarWindowName = "Trackbars";

void on_trackbar(int, void*)
{
}

string intToString(int number) {
	std::stringstream ss;
	ss << number;
	return ss.str();
}

void createTrackbars() {
	char TrackbarName[50];

	createTrackbar("FILTER_FOR", trackbarWindowName, &FILTER_FOR, 2, on_trackbar);

	if (FILTER_FOR == 0) {
		createTrackbar("H_MIN", trackbarWindowName, &POINTER1_H_MIN, H_MAX, on_trackbar);
		createTrackbar("H_MAX", trackbarWindowName, &POINTER1_H_MAX, H_MAX, on_trackbar);
		createTrackbar("S_MIN", trackbarWindowName, &POINTER1_S_MIN, S_MAX, on_trackbar);
		createTrackbar("S_MAX", trackbarWindowName, &POINTER1_S_MAX, S_MAX, on_trackbar);
		createTrackbar("V_MIN", trackbarWindowName, &POINTER1_V_MIN, V_MAX, on_trackbar);
		createTrackbar("V_MAX", trackbarWindowName, &POINTER1_V_MAX, V_MAX, on_trackbar);
	}

	if (FILTER_FOR == 1) {
		createTrackbar("H_MIN", trackbarWindowName, &POINTER2_H_MIN, H_MAX, on_trackbar);
		createTrackbar("H_MAX", trackbarWindowName, &POINTER2_H_MAX, H_MAX, on_trackbar);
		createTrackbar("S_MIN", trackbarWindowName, &POINTER2_S_MIN, S_MAX, on_trackbar);
		createTrackbar("S_MAX", trackbarWindowName, &POINTER2_S_MAX, S_MAX, on_trackbar);
		createTrackbar("V_MIN", trackbarWindowName, &POINTER2_V_MIN, V_MAX, on_trackbar);
		createTrackbar("V_MAX", trackbarWindowName, &POINTER2_V_MAX, V_MAX, on_trackbar);
	}
	if (FILTER_FOR == 2) {
		createTrackbar("H_MIN", trackbarWindowName, &POINTER3_H_MIN, H_MAX, on_trackbar);
		createTrackbar("H_MAX", trackbarWindowName, &POINTER3_H_MAX, H_MAX, on_trackbar);
		createTrackbar("S_MIN", trackbarWindowName, &POINTER3_S_MIN, S_MAX, on_trackbar);
		createTrackbar("S_MAX", trackbarWindowName, &POINTER3_S_MAX, S_MAX, on_trackbar);
		createTrackbar("V_MIN", trackbarWindowName, &POINTER3_V_MIN, V_MAX, on_trackbar);
		createTrackbar("V_MAX", trackbarWindowName, &POINTER3_V_MAX, V_MAX, on_trackbar);
	}
	createTrackbar("Track", trackbarWindowName, &TRACK_STATUS, 1, on_trackbar);
}

void drawObject(int x, int y, Mat &frame) {
	circle(frame, Point(x, y), 10, Scalar(0, 255, 0), 1);
	if (y - 25>0)
		line(frame, Point(x, y), Point(x, y - 25), Scalar(0, 255, 0), 1);
	else line(frame, Point(x, y), Point(x, 0), Scalar(0, 255, 0), 1);
	if (y + 25<FRAME_HEIGHT)
		line(frame, Point(x, y), Point(x, y + 25), Scalar(0, 255, 0), 1);
	else line(frame, Point(x, y), Point(x, FRAME_HEIGHT), Scalar(0, 255, 0), 1);
	if (x - 25>0)
		line(frame, Point(x, y), Point(x - 25, y), Scalar(0, 255, 0), 1);
	else line(frame, Point(x, y), Point(0, y), Scalar(0, 255, 0), 1);
	if (x + 25<FRAME_WIDTH)
		line(frame, Point(x, y), Point(x + 25, y), Scalar(0, 255, 0), 1);
	else line(frame, Point(x, y), Point(FRAME_WIDTH, y), Scalar(0, 255, 0), 1);

	putText(frame, intToString(x) + "," + intToString(y), Point(x, y + 30), 1, 1, Scalar(0, 255, 0), 2);
}

void morphOps(Mat &thresh) {

	Mat erodeElement = getStructuringElement(MORPH_RECT, Size(2, 2));
	Mat dilateElement = getStructuringElement(MORPH_RECT, Size(5, 5));

	erode(thresh, thresh, erodeElement);
	erode(thresh, thresh, erodeElement);

	dilate(thresh, thresh, dilateElement);
	dilate(thresh, thresh, dilateElement);

}

void clickAndDrag_Rectangle(int event, int x, int y, int flags, void* param) {
	
	if (calibrationMode == true) {
	
		Mat* videoFeed = (Mat*)param;

		if (event == CV_EVENT_LBUTTONDOWN && mouseIsDragging == false)
		{
			initialClickPoint = cv::Point(x, y);
			mouseIsDragging = true;
		}
		
		if (event == CV_EVENT_MOUSEMOVE && mouseIsDragging == true)
		{
			currentMousePoint = cv::Point(x, y);
			mouseMove = true;
		}
		
		if (event == CV_EVENT_LBUTTONUP && mouseIsDragging == true)
		{
		
			rectangleROI = Rect(initialClickPoint, currentMousePoint);
			mouseIsDragging = false;
			mouseMove = false;
			rectangleSelected = true;
		}

		if (event == CV_EVENT_RBUTTONDOWN) {
			
			if (FILTER_FOR == 0)
			{
				POINTER1_H_MIN = 0;
				POINTER1_H_MAX = 180;
				POINTER1_S_MIN = 0;
				POINTER1_S_MAX = 256;
				POINTER1_V_MIN = 0;
				POINTER1_V_MAX = 256;
			}
			if (FILTER_FOR == 1)
			{
				POINTER2_H_MIN = 0;
				POINTER2_H_MAX = 180;
				POINTER2_S_MIN = 0;
				POINTER2_S_MAX = 256;
				POINTER2_V_MIN = 0;
				POINTER2_V_MAX = 256;
			}
			if (FILTER_FOR == 2)
			{
				POINTER3_H_MIN = 0;
				POINTER3_H_MAX = 180;
				POINTER3_S_MIN = 0;
				POINTER3_S_MAX = 256;
				POINTER3_V_MIN = 0;
				POINTER3_V_MAX = 256;
			}
		}
		if (event == CV_EVENT_MBUTTONDOWN) {

		}
	}
}

void recordHSV_Values(cv::Mat frame, cv::Mat hsv_frame) {

	if (mouseMove == false && rectangleSelected == true) {

		if (H_ROI.size()>0) H_ROI.clear();
		if (S_ROI.size()>0) S_ROI.clear();
		if (V_ROI.size()>0)V_ROI.clear();
		if (rectangleROI.width<1 || rectangleROI.height<1) cout << "Please drag a rectangle, not a line" << endl;
		else {
			for (int i = rectangleROI.x; i<rectangleROI.x + rectangleROI.width; i++) {
				for (int j = rectangleROI.y; j<rectangleROI.y + rectangleROI.height; j++) {
					H_ROI.push_back((int)hsv_frame.at<cv::Vec3b>(j, i)[0]);
					S_ROI.push_back((int)hsv_frame.at<cv::Vec3b>(j, i)[1]);
					V_ROI.push_back((int)hsv_frame.at<cv::Vec3b>(j, i)[2]);
				}
			}
		}
		rectangleSelected = false;

		if (FILTER_FOR == 0)
		{
			if (H_ROI.size()>0) {
				POINTER1_H_MIN = *std::min_element(H_ROI.begin(), H_ROI.end());
				POINTER1_H_MAX = *std::max_element(H_ROI.begin(), H_ROI.end());
			}
			if (S_ROI.size()>0) {
				POINTER1_S_MIN = *std::min_element(S_ROI.begin(), S_ROI.end());
				POINTER1_S_MAX = *std::max_element(S_ROI.begin(), S_ROI.end());
			}
			if (V_ROI.size()>0) {
				POINTER1_V_MIN = *std::min_element(V_ROI.begin(), V_ROI.end());
				POINTER1_V_MAX = *std::max_element(V_ROI.begin(), V_ROI.end());
			}
		}

		if (FILTER_FOR == 1)
		{
			if (H_ROI.size()>0) {
				POINTER2_H_MIN = *std::min_element(H_ROI.begin(), H_ROI.end());
				POINTER2_H_MAX = *std::max_element(H_ROI.begin(), H_ROI.end());
			}
			if (S_ROI.size()>0) {
				POINTER2_S_MIN = *std::min_element(S_ROI.begin(), S_ROI.end());
				POINTER2_S_MAX = *std::max_element(S_ROI.begin(), S_ROI.end());
			}
			if (V_ROI.size()>0) {
				POINTER2_V_MIN = *std::min_element(V_ROI.begin(), V_ROI.end());
				POINTER2_V_MAX = *std::max_element(V_ROI.begin(), V_ROI.end());
			}
		}
		if (FILTER_FOR == 2)
		{
			if (H_ROI.size()>0) {
				POINTER3_H_MIN = *std::min_element(H_ROI.begin(), H_ROI.end());
				POINTER3_H_MAX = *std::max_element(H_ROI.begin(), H_ROI.end());
			}
			if (S_ROI.size()>0) {
				POINTER3_S_MIN = *std::min_element(S_ROI.begin(), S_ROI.end());
				POINTER3_S_MAX = *std::max_element(S_ROI.begin(), S_ROI.end());
			}
			if (V_ROI.size()>0) {
				POINTER3_V_MIN = *std::min_element(V_ROI.begin(), V_ROI.end());
				POINTER3_V_MAX = *std::max_element(V_ROI.begin(), V_ROI.end());
			}
		}
	}

	if (mouseMove == true) {
		rectangle(frame, initialClickPoint, cv::Point(currentMousePoint.x, currentMousePoint.y), cv::Scalar(0, 255, 0), 1, 8, 0);
	}
}

void trackFilteredPointers(Mat threshold1, Mat threshold2, Mat threshold3, Mat &cameraFeed) {

	Mat temp1;
	Mat temp2;
	Mat temp3;
	threshold1.copyTo(temp1);
	threshold2.copyTo(temp2);
	threshold3.copyTo(temp3);
	vector< vector<Point> > contours1;
	vector< vector<Point> > contours2;
	vector< vector<Point> > contours3;
	vector<Vec4i> hierarchy1;
	vector<Vec4i> hierarchy2;
	vector<Vec4i> hierarchy3;

	findContours(temp1, contours1, hierarchy1, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
	findContours(temp2, contours2, hierarchy2, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
	findContours(temp3, contours3, hierarchy3, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

	double refArea1 = 0;
	double refArea2 = 0;
	double refArea3 = 0;

	if (hierarchy1.size() > 0) {
		int numObjects = hierarchy1.size();
		if (numObjects<MAX_NUM_OBJECTS) {
			for (int index = 0; index >= 0; index = hierarchy1[index][0]) {

				Moments moment = moments((cv::Mat)contours1[index]);
				double area = moment.m00;
				if (area>MIN_OBJECT_AREA && area<MAX_OBJECT_AREA && area>refArea1) {
					Px1 = moment.m10 / area;
					Py1 = moment.m01 / area;
					POINTER1_FOUND = true;
					drawContours(cameraFeed, contours1, 0, Scalar(255, 0, 0), 0, 8, hierarchy1);
					putText(cameraFeed, "Tracking First pointer", Point(2, 15), 1, 1, Scalar(0, 255, 0), 2);
					drawObject(Px1, Py1, cameraFeed);
					refArea1 = area;
				}
				else POINTER1_FOUND = false;
			}
		}
		else putText(cameraFeed, "Too much noise in first pointer ! Adjust Filter", Point(2, 15), 1, 1, Scalar(0, 0, 255), 2);
	}
	
	if (hierarchy2.size() > 0) {
		int numObjects = hierarchy2.size();
		if (numObjects<MAX_NUM_OBJECTS) {
			for (int index = 0; index >= 0; index = hierarchy2[index][0]) {

				Moments moment = moments((cv::Mat)contours2[index]);
				double area = moment.m00;
				if (area>MIN_OBJECT_AREA && area<MAX_OBJECT_AREA && area>refArea2) {
					Px2 = moment.m10 / area;
					Py2 = moment.m01 / area;
					POINTER2_FOUND = true;
					putText(cameraFeed, "Tracking Second Pointer", Point(2, 30), 1, 1, Scalar(0, 255, 0), 2);
					drawContours(cameraFeed, contours2, 0, Scalar(0, 255, 0), 0, 8, hierarchy2);
					refArea2 = area;
				}
				else POINTER2_FOUND = false;
			}
		}
		else putText(cameraFeed, "Too much noise in second pointer! Adjust Filter", Point(2, 30), 1, 1, Scalar(0, 0, 255), 2);
	}
	
	if (hierarchy3.size() > 0) {
		int numObjects = hierarchy3.size();
		if (numObjects<MAX_NUM_OBJECTS) {
			for (int index = 0; index >= 0; index = hierarchy3[index][0]) {

				Moments moment = moments((cv::Mat)contours3[index]);
				double area = moment.m00;
				if (area>MIN_OBJECT_AREA && area<MAX_OBJECT_AREA && area>refArea3) {
					Px3 = moment.m10 / area;
					Py3 = moment.m01 / area;
					POINTER3_FOUND = true;
					putText(cameraFeed, "Tracking Third Pointer", Point(2, 45), 1, 1, Scalar(0, 255, 0), 2);
					drawContours(cameraFeed, contours3, 0, Scalar(0, 255, 0), 0, 8, hierarchy3);
					refArea3 = area;
				}
				else POINTER3_FOUND = false;
			}
		}
		else putText(cameraFeed, "Too much noise in third pointer! Adjust Filter", Point(2, 45), 1, 1, Scalar(0, 0, 255), 2);
	}
}

int main(int argc, char* argv[])
{
	#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
	bool trackObjects = true;
	bool useMorphOps = true;
	calibrationMode = true;

	Mat cameraFeed;
	Mat HSV;
	Mat POINTER1_threshold;
	Mat POINTER2_threshold;
	Mat POINTER3_threshold;

	VideoCapture capture;

	capture.open(0);

	capture.set(CV_CAP_PROP_FRAME_WIDTH, FRAME_WIDTH);
	capture.set(CV_CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT);
	
	cv::namedWindow(windowName);
	cv::setMouseCallback(windowName, clickAndDrag_Rectangle, &cameraFeed);
	cv::namedWindow(trackbarWindowName);
	cv::resizeWindow(trackbarWindowName, 300, 350);

	mouseIsDragging = false;
	mouseMove = false;
	rectangleSelected = false;

	while (waitKey(30)!= 27) {

		capture.read(cameraFeed);
		cv::flip(cameraFeed, cameraFeed, 1);
		cvtColor(cameraFeed, HSV, COLOR_BGR2HSV);
		recordHSV_Values(cameraFeed, HSV);

		inRange(HSV, Scalar(POINTER1_H_MIN, POINTER1_S_MIN, POINTER1_V_MIN), Scalar(POINTER1_H_MAX, POINTER1_S_MAX, POINTER1_V_MAX), POINTER1_threshold);
		morphOps(POINTER1_threshold);
		
		inRange(HSV, Scalar(POINTER2_H_MIN, POINTER2_S_MIN, POINTER2_V_MIN), Scalar(POINTER2_H_MAX, POINTER2_S_MAX, POINTER2_V_MAX), POINTER2_threshold);
		morphOps(POINTER2_threshold);

		inRange(HSV, Scalar(POINTER3_H_MIN, POINTER3_S_MIN, POINTER3_V_MIN), Scalar(POINTER3_H_MAX, POINTER3_S_MAX, POINTER3_V_MAX), POINTER3_threshold);
		morphOps(POINTER3_threshold);

		if (FILTER_FOR == 0)
		{
			imshow("First Pointer Filter", POINTER1_threshold);
			destroyWindow("Second Pointer Filter");
			destroyWindow("Third Pointer Filter");
			moveWindow("First Pointer Filter", FRAME_WIDTH, 0);
		}

		if (FILTER_FOR == 1)
		{
			imshow("Second Pointer Filter", POINTER2_threshold);
			destroyWindow("First Pointer Filter");
			destroyWindow("Third Pointer Filter");
			moveWindow("Second Pointer Filter", FRAME_WIDTH, 0);
		}
		
		if (FILTER_FOR == 2)
		{
			imshow("Third Pointer Filter", POINTER3_threshold);
			destroyWindow("First Pointer Filter");
			destroyWindow("Second Pointer Filter");
			moveWindow("Third Pointer Filter", FRAME_WIDTH, 0);
		}
		
		trackFilteredPointers(POINTER1_threshold, POINTER2_threshold, POINTER3_threshold, cameraFeed);
		
		cv::Point coordinates;

		coordinates.x = int(USER_SCREEN_WIDTH * Px1 / 640.0);
		coordinates.y = int(USER_SCREEN_HEIGHT * Py1 / 480.0);

		if (TRACK_STATUS) {
			calibrationMode = false;
			if (POINTER1_FOUND)
			{
				//putText(cameraFeed, intToString(RCLICK) + "hi", Point(2, 120), 1, 1, Scalar(0, 255, 0), 2);
				SetCursorPos(coordinates.x, coordinates.y);
				putText(cameraFeed, "x" + intToString(coordinates.x), Point(2, 50), 1, 1, Scalar(0, 255, 0), 2);
				putText(cameraFeed, "y" + intToString(coordinates.y), Point(2, 80), 1, 1, Scalar(0, 255, 0), 2);
				
				if (POINTER2_FOUND)
				{
					
					if (!LCLICK)
					{
						mouse_event(MOUSEEVENTF_LEFTDOWN, coordinates.x, coordinates.y, 0, 0);
						putText(cameraFeed, "Performing L_Click", Point(2, 120), 1, 1, Scalar(0, 255, 0), 2);
						LCLICK = true;
					}

				}

				if (!POINTER2_FOUND)
				{
					if (LCLICK)
					{
						mouse_event(MOUSEEVENTF_LEFTUP, coordinates.x, coordinates.y, 0, 0);
						putText(cameraFeed, "Removing L_Click", Point(2, 120), 1, 1, Scalar(0, 255, 0), 2);
						LCLICK = false;
					}
				}
				
				if (POINTER3_FOUND)
				{

					if (!RCLICK)
					{
						mouse_event(MOUSEEVENTF_RIGHTDOWN, coordinates.x, coordinates.y, 0, 0);
						putText(cameraFeed, "Performing R_Click", Point(2, 120), 1, 1, Scalar(0, 255, 0), 2);
						RCLICK = true;
					}

				}

				if (!POINTER3_FOUND)
				{
					if (RCLICK)
					{
						mouse_event(MOUSEEVENTF_RIGHTUP, coordinates.x, coordinates.y, 0, 0);
						putText(cameraFeed, "Removing R_Click", Point(2, 120), 1, 1, Scalar(0, 255, 0), 2);
						RCLICK = false;
					}
				}

			}

			
			
		}

		else {
			calibrationMode = true;
		}

		imshow(windowName, cameraFeed);
		moveWindow(windowName, 0, 0);
		
		createTrackbars();
		//if (waitKey(30) == 99) calibrationMode = !calibrationMode;
	}
	return 0;
}
