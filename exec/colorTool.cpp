#include "colorRecog.h"
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

static void onMouseHandle(int event, int x, int y, int flags, void *param);
static void take_photo(Mat &img);
static void adjHsvValue();  //tool 1

//-------------------------------------------------------------
int main(int argc, char **argv)
{
    cout << "enter number to start specific function:\n"
            "1. adjust hsv value\n";

    int choice = 1;
    cin >> choice;
    switch (choice)
    {
    case 1:
        adjHsvValue();
    }
    
    return 0;
}

static void onMouseHandle(int event, int x, int y, int flags, void* param)
{
    switch(event)
    {
    case EVENT_LBUTTONDOWN:
        cout << "(" << x << "," << y << ")" << endl;
        break;
    }
}

static void take_photo(Mat &img)
{
    const string photo_path = "./";
    // get system time as file name..
    time_t tt;
    time(&tt);
    tt += 8 * 3600; //transform the time zone
    tm *t = gmtime(&tt);
    char timeBuff[30];
    sprintf(timeBuff, "%d-%02d-%02d %02d-%02d-%02d",
            t->tm_year + 1900,
            t->tm_mon + 1,
            t->tm_mday,
            t->tm_hour,
            t->tm_min,
            t->tm_sec);

    string img_path = photo_path + timeBuff + ".jpg";
    imwrite(img_path, img);
}

static void adjHsvValue()
{
    //get video objects
    int camIdx[2];
    getOrderCamera(camIdx);
    //
    VideoCapture downCam(camIdx[0]);
    VideoCapture upCam(camIdx[1]);

    namedWindow("Control", WINDOW_AUTOSIZE);
    namedWindow("Thresholded Image 1", WINDOW_AUTOSIZE);
    namedWindow("Thresholded Image 2", WINDOW_AUTOSIZE);
    namedWindow("Original 1", WINDOW_AUTOSIZE);
    namedWindow("Original 2", WINDOW_AUTOSIZE);
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
    setMouseCallback("Original 1", onMouseHandle, nullptr);
    setMouseCallback("Original 2", onMouseHandle, nullptr);
    setMouseCallback("perspective 1", onMouseHandle, nullptr);
    setMouseCallback("perspective 2", onMouseHandle, nullptr);

    // Mat imgOriginal1 = imread("../data/up2.jpg");
    // Mat imgOriginal2 = imread("../data/down2.jpg");
    Mat imgOriginal1, imgOriginal2;

    //perspective transform
    vector<Point2f>
        quadPtSrc{Point2f(538, 254), Point2f(351, 196), Point2f(294, 61), Point2f(464, 119)};
    vector<Point2f> quadPtDst{Point2f(0, 0), Point2f(100, 0), Point2f(100, 100), Point2f(0, 100)};
    Mat transmtx = getPerspectiveTransform(quadPtSrc, quadPtDst);

    while (waitKey(30) != 'q')
    {
        downCam >> imgOriginal1;
        upCam >> imgOriginal2;

        Mat imgHSV1, imgHSV2;
        Mat imgPers1, imgPers2;
        vector<Mat> hsvSplit1, hsvSplit2;
        cvtColor(imgOriginal1, imgHSV1, COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV
        cvtColor(imgOriginal2, imgHSV2, COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV

        //因为我们读取的是彩色图，直方图均衡化需要在HSV空间做
        split(imgHSV1, hsvSplit1);
        equalizeHist(hsvSplit1[2], hsvSplit1[2]);
        merge(hsvSplit1, imgHSV1);
        split(imgHSV2, hsvSplit2);
        equalizeHist(hsvSplit2[2], hsvSplit2[2]);
        merge(hsvSplit2, imgHSV2);

        Mat imgThresholded1, imgThresholded2;
        inRange(imgHSV1, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded1); //Threshold the image
        inRange(imgHSV2, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded2); //Threshold the image

        //开操作 (去除一些噪点), 闭操作 (连接一些连通域)
        Mat element = getStructuringElement(MORPH_RECT, Size(5, 5));
        morphologyEx(imgThresholded1, imgThresholded1, MORPH_OPEN, element);
        morphologyEx(imgThresholded1, imgThresholded1, MORPH_CLOSE, element);
        morphologyEx(imgThresholded2, imgThresholded2, MORPH_OPEN, element);
        morphologyEx(imgThresholded2, imgThresholded2, MORPH_CLOSE, element);

        warpPerspective(imgThresholded1, imgPers1, transmtx, Size(100, 100));

        imshow("Thresholded Image 1", imgThresholded1);
        imshow("Original 1", imgOriginal1);
        imshow("Thresholded Image 2", imgThresholded2);
        imshow("Original 2", imgOriginal2);
        imshow("perspective", imgPers1);
    }
}

