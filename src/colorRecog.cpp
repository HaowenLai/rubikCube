/* ********************************************************
*   This is the realization of header colorRecog.h

* @Author : Derek Lai
* @Date   : 2018/11/11
* *******************************************************/
#include "colorRecog.h"

#include <stdio.h>
#include <string.h>
#include <fcntl.h> /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>

using namespace std;
using namespace cv;

static int device_open(const char *dev_name);
static int device_close(int fd);
static void device_busInfo(char *bus_info_actual, int fd);
static char readBlockColor(cv::Mat srcImg);

//---------------------------- public function ------------------------------

//  this functoin get ordered camera and put their index into `rtCamIndex[]`
//rtCamIndex[0] is the index of down camera, rtCamIndex[1] is the index of
//up camera.
void getOrderCamera(int rtCamIndex[])
{
    const int dev_num = 2;
    const char dev_list[][15] = {"/dev/video0", "/dev/video1"};
    const char bus_info[][25] = {"usb-0000:00:14.0-4", "usb-0000:00:1d.0-1.6"};

    //get actual camera bus info
    char bus_info_actual[dev_num][25];
    for (int i = 0; i < dev_num; ++i)
    {
        int fd = device_open(dev_list[i]);
        device_busInfo(bus_info_actual[i], fd);
        device_close(fd);
    }

    //return ordered index
    for (int i = 0; i < dev_num; i++)
    {
        int index = 0;
        while (!strcmp(bus_info[i], bus_info_actual[index]))
            index++;

        rtCamIndex[i] = index;
    }
}


//  get steady state up and down image for later analyzation.
//By default it skip the first 20 frames,
void getUpDowmImg(VideoCapture &downCam, VideoCapture &upCam,
                  Mat &downImg, Mat &upImg,
                  int skipFrame)
{
    for (int i = 0; i < skipFrame; i++)
    {
        downCam >> downImg;
        upCam >> upImg;
        waitKey(20);
    }
}

// Read all six faces colors.
//Input images from up and down cameras, and output the color letter array.
//The output rank is the same as that defined in class `RubikCube`.
//e.g. "rgbowyyoow....."
void read6faceColor(cv::Mat &upImg, cv::Mat &downImg, char colorLetter[])
{
    //The rank is up, left, front, right, back, down. ULFRBD.
    const vector<vector<Point2f>> facesQuadPtSrc{
        vector<Point2f>{Point2f(538, 254), Point2f(351, 196), Point2f(294, 61), Point2f(464, 119)},  //up
        vector<Point2f>{Point2f(483, 122), Point2f(540, 304), Point2f(328, 242), Point2f(290, 53)},  //left
        vector<Point2f>{Point2f(538, 313), Point2f(390, 441), Point2f(191, 395), Point2f(323, 251)}, //front
        vector<Point2f>{Point2f(289, 63), Point2f(347, 195), Point2f(229, 369), Point2f(197, 217)},  //right
        vector<Point2f>{Point2f(350, 201), Point2f(538, 267), Point2f(416, 406), Point2f(232, 369)}, //back
        vector<Point2f>{Point2f(319, 244), Point2f(183, 391), Point2f(174, 199), Point2f(280, 56)}}; //down
    const vector<Point2f> facesQuadPtDst{Point2f(0, 0), Point2f(100, 0), Point2f(100, 100), Point2f(0, 100)};
    const char centerColor[]{'w', 'r', 'b', 'o', 'g', 'y'};

    Mat upHSV, downHSV;
    cvtColor(upImg, upHSV, COLOR_BGR2HSV);
    cvtColor(downImg, downHSV, COLOR_BGR2HSV);

    //equalize histogram to balance brightness
    vector<Mat> upHsvSplit, downHsvSplit;
    split(upHSV, upHsvSplit);
    split(downHSV, downHsvSplit);
    equalizeHist(upHsvSplit[2], upHsvSplit[2]);
    equalizeHist(downHsvSplit[2], downHsvSplit[2]);
    merge(upHsvSplit, upHSV);
    merge(downHsvSplit, downHSV);

    //perform perspective transformation.
    //Then, read color from each single block
    Mat *upDownHsvPt[]{&upHSV, &downHSV, &downHSV, &upHSV, &upHSV, &downHSV};
    for(int faceIdx = 0; faceIdx < 6; faceIdx++)
    {
        Mat imgPers;
        Mat transmtx = getPerspectiveTransform(facesQuadPtSrc[faceIdx], facesQuadPtDst);
        warpPerspective(*(upDownHsvPt[faceIdx]), imgPers, transmtx, Size(100, 100));
        
        for (int blockIdx = 0; blockIdx < 9; blockIdx++)
        {
            //assign central block color directly
            if(blockIdx == 4)
            {
                colorLetter[9 * faceIdx + blockIdx] = centerColor[faceIdx];
                continue;
            }
            
            Mat ROI = imgPers(Rect(blockIdx % 3 * 33, blockIdx / 3 * 33, 33, 33));
            colorLetter[9 * faceIdx + blockIdx] = readBlockColor(ROI);
        }
    }
}

//------------------------------- static function ---------------------------

static int device_open(const char *dev_name)
{
    struct stat st;
    if (-1 == stat(dev_name, &st))
    {
        fprintf(stderr, "Cannot identify '%s': %d, %s\n", dev_name, errno, strerror(errno));
        throw - 1;
    }

    if (!S_ISCHR(st.st_mode))
    {
        fprintf(stderr, "%s is no device\n", dev_name);
        throw - 1;
    }

    int fd = open(dev_name, O_RDWR /* required */ | O_NONBLOCK, 0);
    if (-1 == fd)
    {
        fprintf(stderr, "Cannot open '%s': %d, %s\n", dev_name, errno, strerror(errno));
        throw - 1;
    }

    // printf("%s %d : Open %s successfully. fd = %d\n", __func__, __LINE__, dev_name, fd);
    return fd;
}

static int device_close(int fd)
{
    if (-1 == close(fd))
    {
        printf("\tdevice close failed.\n");
        throw - 1;
    }
    // else
    //     printf("%s %d : devices close successfully\n\n", __func__, __LINE__);

    return 0;
}

static void device_busInfo(char *bus_info_actual, int fd)
{
    struct v4l2_capability cap;
    memset(&cap, 0, sizeof(cap));

    if (-1 == ioctl(fd, VIDIOC_QUERYCAP, &cap)) // query v4l2-devices's capability
    {
        if (EINVAL == errno)
        {
            fprintf(stderr, "device of fd %d is no V4L2 device\n", fd);
            throw - 1;
        }
        else
        {
            printf("\tvideo ioctl_querycap failed.\n");
            throw - 1;
        }
    }

    // printf("\n\tdriver name : %s\n\tcard name : %s\n\tbus info : %s\n\tdriver version : %u.%u.%u\n\n",
    //        cap.driver,
    //        cap.card,
    //        cap.bus_info,
    //        (cap.version >> 16) & 0XFF,
    //        (cap.version >> 8) & 0XFF,
    //        cap.version & 0XFF);
    strcpy(bus_info_actual, (char *)cap.bus_info);
}

//  read and indentify the color of `srcImg` which is the image of a single block.
//`srcImg` : HSV 3-channels block image.
//The function return a char letter representing the color.
//e.g.  'y' for Yellow, 'r' for Red, 'o' for Orange. etc.
static char readBlockColor(cv::Mat srcImg)
{
    // rank is                               white,              red,                blue,
    //                                       orange,             green,              yellow
    const vector<Scalar> colorRangeLow{Scalar(50, 1, 70), Scalar(163, 58, 40), Scalar(95, 85, 70),
                                       Scalar(0, 33, 33), Scalar(52, 60, 40), Scalar(35, 40, 50)};
    const vector<Scalar> colorRangeHigh{Scalar(179, 31, 255), Scalar(179, 180, 255), Scalar(110, 250, 255),
                                        Scalar(10, 200, 255), Scalar(70, 180, 255), Scalar(50, 180, 255)};

    //used to count the pixel of each color.
    const char colorSymbols[]{'w', 'r', 'b', 'o', 'g', 'y'};
    float colorCount[6];

    //preparation for calcHist()
    const int channels[] = {0};
    const int histSize[] = {2};
    const float hrange[] = {.0f, 256.0f};
    const float *ranges[] = {hrange};

    //start counting
    Mat element = getStructuringElement(MORPH_RECT, Size(5, 5));
    for (int i = 0; i < 6; i++)
    {
        Mat imgThresholded;
        inRange(srcImg, colorRangeLow[i], colorRangeHigh[i], imgThresholded);

        //open operation and close operation
        morphologyEx(imgThresholded, imgThresholded, MORPH_OPEN, element);
        morphologyEx(imgThresholded, imgThresholded, MORPH_CLOSE, element);

        MatND hist;
        calcHist(&imgThresholded, 1, channels, Mat(), hist, 1, histSize, ranges);
        colorCount[i] = hist.at<float>(1);
    }

    //find the most possible color
    float maxTemp = .0f;
    int maxIndex = 0;
    for (int i = 0; i < 6; i++)
    {
        if (colorCount[i] > maxTemp)
        {
            maxTemp = colorCount[i];
            maxIndex = i;
        }
    }
    return colorSymbols[maxIndex];
}
