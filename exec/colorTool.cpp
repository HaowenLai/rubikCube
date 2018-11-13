#include "colorRecog.h"
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

static void onMouseHandle(int event, int x, int y, int flags, void* param)
{
    switch(event)
    {
    case EVENT_LBUTTONDOWN:
        cout << "(" << x << "," << y << ")" << endl;
        break;
    }
}

int main(int argc, char **argv)
{
    namedWindow("Control", WINDOW_AUTOSIZE);
    namedWindow("Thresholded Image", WINDOW_AUTOSIZE);
    namedWindow("Original", WINDOW_AUTOSIZE);
    namedWindow("perspective", WINDOW_AUTOSIZE);

    //threshold variables
    int iLowH = 59;
    int iHighH = 70;
    //
    int iLowS = 130;
    int iHighS = 221;
    //
    int iLowV = 40;
    int iHighV = 255;

    //Create trackbars in "Control" window
    cvCreateTrackbar("LowH", "Control", &iLowH, 179); //Hue (0 - 179)
    cvCreateTrackbar("HighH", "Control", &iHighH, 179);
    //
    cvCreateTrackbar("LowS", "Control", &iLowS, 255); //Saturation (0 - 255)
    cvCreateTrackbar("HighS", "Control", &iHighS, 255);
    //
    cvCreateTrackbar("LowV", "Control", &iLowV, 255); //Value (0 - 255)
    cvCreateTrackbar("HighV", "Control", &iHighV, 255);
    //set mouse callback
    setMouseCallback("Original", onMouseHandle, nullptr);
    setMouseCallback("perspective", onMouseHandle, nullptr);

    Mat imgOriginal = imread("../data/up1.jpg");

    //perspective transform
    vector<Point2f> quadPtSrc{Point2f(538, 254), Point2f(351, 196), Point2f(294, 61), Point2f(464, 119)};
    vector<Point2f> quadPtDst{Point2f(0, 0), Point2f(100, 0), Point2f(100, 100), Point2f(0, 100)};
    Mat transmtx = getPerspectiveTransform(quadPtSrc, quadPtDst);

    while(waitKey(30)!='q')
    {
        Mat imgHSV, imgPers;
        vector<Mat> hsvSplit;
        cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV

        //因为我们读取的是彩色图，直方图均衡化需要在HSV空间做
        split(imgHSV, hsvSplit);
        equalizeHist(hsvSplit[2], hsvSplit[2]);
        merge(hsvSplit, imgHSV);

        Mat imgThresholded;
        inRange(imgHSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded); //Threshold the image

        //开操作 (去除一些噪点), 闭操作 (连接一些连通域)
        Mat element = getStructuringElement(MORPH_RECT, Size(5, 5));
        morphologyEx(imgThresholded, imgThresholded, MORPH_OPEN, element);
        morphologyEx(imgThresholded, imgThresholded, MORPH_CLOSE, element);

        warpPerspective(imgThresholded, imgPers, transmtx, Size(100, 100));

        imshow("Thresholded Image", imgThresholded);
        imshow("Original", imgOriginal);            
        imshow("perspective", imgPers);
    }

    return 0;

}
