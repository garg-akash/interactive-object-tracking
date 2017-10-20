#include<iostream>
#include<sstream>
using namespace std;

#include <opencv2/core/core.hpp>
#include"opencv2/highgui/highgui.hpp"
//#include"opencv2/videoio.hpp"
#include"opencv2/video/video.hpp"
#include"opencv2/imgproc/imgproc.hpp"
#include <opencv2/opencv.hpp>
using namespace cv;

Point originPoint;
Rect selectedRect, trackingRect;
bool selectRegion= false;
int trackingFlag= 0;
Mat image;

void onMouse(int event, int x, int y, int, void*)
{
	if(selectRegion)
	{
		selectedRect.x = MIN(x, originPoint.x);
		selectedRect.y = MIN(y, originPoint.y);
		selectedRect.width = std::abs(x - originPoint.x);
		selectedRect.height = std::abs(y - originPoint.y);
		selectedRect &= Rect(0, 0, image.cols, image.rows);
	}
	switch(event)
	{
		case CV_EVENT_LBUTTONDOWN:
		originPoint = Point(x,y);
		selectedRect = Rect(x,y,0,0);
		selectRegion = true;
		break;
		case CV_EVENT_LBUTTONUP:
		selectRegion = false;
		if( selectedRect.width > 0 && selectedRect.height > 0 )
		{
			trackingFlag = -1;
		}
		break;
	}
}
int vmin = 10, vmax = 256, smin = 30;
int main(int argc, const char** argv)
{
	VideoCapture cap(0);
	Mat frame, hsv, mask, hue, hist, backproj;
	int hsize= 16;
	float hue_range[] = { 0, 180 };
    const float* ranges = { hue_range };
	namedWindow("Result");
	setMouseCallback( "Result", onMouse, NULL );
	while(true)
	{
		cap>>frame;
		frame.copyTo(image);
		cvtColor(image, hsv, CV_BGR2HSV);
		if(trackingFlag)
		{
			inRange(hsv, Scalar(0, smin, vmin), Scalar(180, 256, vmax), mask);
			int channels[]= {0, 0};
			hue.create(hsv.size(), hsv.depth());
			mixChannels(&hsv, 1, &hue, 1, channels, 1);
			if(trackingFlag< 0)
			{
				Mat roi(hue, selectedRect), maskroi(mask, selectedRect);
				calcHist(&roi, 1, 0, maskroi, hist, 1, &hsize, &ranges);
				normalize(hist, hist, 0, 255, CV_MINMAX);

				trackingRect= selectedRect;
				trackingFlag= 1;
			}
			calcBackProject(&hue, 1, 0, hist, backproj, &ranges);
			backproj &= mask;
			RotatedRect rotatedTrackingRect= CamShift(backproj, trackingRect, TermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 0)); 
			ellipse(image, rotatedTrackingRect, Scalar(255, 0, 0), 3, CV_AA);
		}
		imshow("Result", image);
		char ch= waitKey(30);
		if(ch== 27)
			break;
	}
	return 0;
}

