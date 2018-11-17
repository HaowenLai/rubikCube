/* ********************************************************
*   This is the realization of header colorRecog.h

* @Author : Derek Lai
* @Date   : 2018/11/11
* *******************************************************/
#include "colorRecog.h"
#include "RubikCube.h"

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

//some parameters that used commonly
namespace clrRecog
{
    //usb bus info used to get ordered cameras
    const char bus_info[][25] = {"usb-0000:00:14.0-4", "usb-0000:00:1d.0-1.6"}; //down, up

    //up camera perspective quad points ---  up, right, back
    const vector<vector<Point2f>> quadPtSrcUp{
        vector<Point2f>{Point2f(564, 249), Point2f(363, 188), Point2f(305, 56), Point2f(482, 116)},
        vector<Point2f>{Point2f(299, 58), Point2f(355, 192), Point2f(241, 377), Point2f(205, 224)},
        vector<Point2f>{Point2f(368, 198), Point2f(558, 261), Point2f(437, 416), Point2f(248, 380)}};

    //down camera perspective quad points ---  left, front, down
    const vector<vector<Point2f>> quadPtSrcDn{
        vector<Point2f>{Point2f(526, 146), Point2f(572, 317), Point2f(345, 219), Point2f(335, 46)},
        vector<Point2f>{Point2f(562, 332), Point2f(415, 453), Point2f(206, 390), Point2f(341, 235)},
        vector<Point2f>{Point2f(331, 217), Point2f(195, 377), Point2f(208, 201), Point2f(318, 53)}};

    //destiny perspective quad points
    const vector<Point2f> quadPtDst{Point2f(0, 0), Point2f(150, 0), Point2f(150, 150), Point2f(0, 150)};

    //The location points of perspective faces
    const vector<Point> locPtUp{Point(170, 10), Point(330, 170), Point(490, 170)}; //up, right, back
    const vector<Point> locPtDn{Point(10, 170), Point(170, 170), Point(170, 330)}; //left, front, down

    //expel block(cannot see that color) number. Start from 0.
    const int expelBlkNoUp[]{6, 6, 8}; //up, right, back
    const int expelBlkNoDn[]{0, 2, 8}; //left, front, down

    //block start index to get correct color letters labels
    const int blkStIdxUp[]{0, 21, 28}; //up, right, back
    const int blkStIdxDn[]{7, 14, 35}; //left, front, down

    //color letter offset. It is used to put result into the return array
    const int colorLtOffsUp[]{0, 27, 36}; //up, right, back
    const int colorLtOffsDn[]{9, 18, 45}; //left, front, down
};


//---------------------------- public function ------------------------------

//  this functoin get ordered camera and put their index into `rtCamIndex[]`
//rtCamIndex[0] is the index of down camera, rtCamIndex[1] is the index of
//up camera.
void getOrderCamera(int rtCamIndex[])
{
    using clrRecog::bus_info;

    const int dev_num = 2;
    const char dev_list[][15] = {"/dev/video0", "/dev/video1"};

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


// Infer the unknown color of an angle-block.
//For every angle-block, the color unknown can be infered if the rest two colors are clear.
//Colors are represented by letters 'wrbogy', and the unknow is represented by 'x'.
//NOTE: up-w, left-r, front-b
void inferUnknownClr(char colorLetters[])
{
    //look up table for unknown color.  [idx0][idx1]
    //idx0: unknown color index, the letter is in `colorRank`
    //idx1: put the unknown color upward, the rest 4 colors listed
    //      conter-clockwisely.
    const char lookUpTb[6][4]{{'r', 'b', 'o', 'g'},
                              {'w', 'g', 'y', 'b'},
                              {'y', 'o', 'w', 'r'},
                              {'y', 'g', 'w', 'b'},
                              {'y', 'r', 'w', 'o'},
                              {'b', 'r', 'g', 'o'}};
    const char colorRank[]{'w', 'r', 'b', 'o', 'g', 'y'};
    //
    //unknown color index in `colorLetters[]`
    const int unknownClrIdx[]{6, 9, 20, 33, 44, 53};
    //put the unknown color upward, the index of rest 2 color conter-clockwisely
    const int rest2ClrConterIdx[6][2]{{11, 18}, {0, 38},  {27, 8}, 
                                      {26, 47}, {51, 15}, {42, 35}};

    //start to deal with unknown colors..
    //each unknown color
    for (int i = 0; i < 6; i++)
    {
        char lt1 = colorLetters[rest2ClrConterIdx[i][0]];
        char lt2 = colorLetters[rest2ClrConterIdx[i][1]];
        bool breakFlag = false;

        //each line of look up table
        for (int j = 0; j < 6; j++)
        {
            //check if the current j-th line is right
            for (int k = 0; k < 4;k++)
            {
                if (lookUpTb[j][k] == lt1 && lookUpTb[j][(k + 1) % 4] == lt2)
                {
                    colorLetters[unknownClrIdx[i]] = colorRank[j];
                    breakFlag = true;
                    break;
                }
            }

            if(breakFlag)
                break;
        }//end line of lookupTb
    }//end i
}

// train svm models for up and down cameras respectively to recognize colors.
void trainColorModel(cv::VideoCapture &downCam, cv::VideoCapture &upCam,
                     std::string modelPath)
{
    using namespace clrRecog;

    Mat upImg, downImg;
    Mat background(500, 650, CV_8UC3, Scalar(200, 200, 200));

    int initBlkGapX = 7;
    int initBlkGapY = 10;
    int blkGap = 28;
    int blkSize = 25;

    namedWindow("original up", WINDOW_AUTOSIZE);
    namedWindow("original down", WINDOW_AUTOSIZE);
    namedWindow("trans", WINDOW_AUTOSIZE);
    namedWindow("Control", WINDOW_AUTOSIZE);
    //
    cvCreateTrackbar("initBlkGapX", "Control", &initBlkGapX, 15);
    cvCreateTrackbar("initBlkGapY", "Control", &initBlkGapY, 15);
    cvCreateTrackbar("blkGap", "Control", &blkGap, 35);
    cvCreateTrackbar("blkSize", "Control", &blkSize, 30);

    //preparation for SVM
    Mat svmDataUp, svmLabelsUp;
    Mat svmDataDn, svmLabelsDn;
    char inputLabels[60];

    //aquire training samples
    cout << "press 'r' to record train samples, 's' to start training..\n";
    while (1)
    {
        upCam >> upImg;
        downCam >> downImg;

        char key = waitKey(30);
        if (key == 's')
            break;
        else if (key == 'r')
        {
            cout << "please enter the color letters: ";
            cin >> inputLabels;
            cout << "samples recorded.\n\n";
        }

        //deal with the up camera
        for (int i = 0; i < 3; i++)
        {
            Mat persImg;
            Mat transmtx = getPerspectiveTransform(quadPtSrcUp[i], quadPtDst);
            warpPerspective(upImg, persImg, transmtx, Size(150, 150));

            //extract block color
            int blkCount = blkStIdxUp[i];
            for (int j = 0; j < 9; j++)
            {
                if (j == expelBlkNoUp[i] || j == 4)
                    continue;

                int x = j % 3;
                int y = j / 3;
                Rect extractArea((blkGap + blkSize) * x + initBlkGapX, (blkGap + blkSize) * y + initBlkGapY, blkSize, blkSize);

                if (key == 'r')
                {
                    Mat blkTemp = persImg(extractArea).clone();
                    cvtColor(blkTemp, blkTemp, COLOR_BGR2HSV);

                    //normalize
                    for (int k = 0; k < blkSize * blkSize; k++)
                    {
                        float x[2];
                        x[0] = blkTemp.at<Vec3b>(k)[0] / 179.0f;
                        x[1] = blkTemp.at<Vec3b>(k)[1] / 255.0f;

                        //deal with data and labels
                        svmDataUp.push_back(Mat(1, 2, CV_32FC1, x));
                        svmLabelsUp.push_back((int)inputLabels[blkCount]);
                    }
                    blkCount++;
                }
                else
                    rectangle(persImg, extractArea, Scalar(0, 0, 0));
            }
            persImg.copyTo(background(Rect(locPtUp[i], Size(150, 150))));
        }

        //deal with the down camera
        for (int i = 0; i < 3; i++)
        {
            Mat persImg;
            Mat transmtx = getPerspectiveTransform(quadPtSrcDn[i], quadPtDst);
            warpPerspective(downImg, persImg, transmtx, Size(150, 150));

            //extract block color
            int blkCount = blkStIdxDn[i];
            for (int j = 0; j < 9; j++)
            {
                if (j == expelBlkNoDn[i] || j == 4)
                    continue;

                int x = j % 3;
                int y = j / 3;
                Rect extractArea((blkGap + blkSize) * x + initBlkGapX, (blkGap + blkSize) * y + initBlkGapY, blkSize, blkSize);

                if (key == 'r')
                {
                    Mat blkTemp = persImg(extractArea).clone();
                    cvtColor(blkTemp, blkTemp, COLOR_BGR2HSV);
                    // blkTemp.reshape(0, blkSize * blkSize).convertTo(blkTemp, CV_32FC3);

                    //normalize
                    for (int k = 0; k < blkSize * blkSize; k++)
                    {
                        float x[2];
                        x[0] = blkTemp.at<Vec3b>(k)[0] / 179.0f;
                        x[1] = blkTemp.at<Vec3b>(k)[1] / 255.0f;

                        //deal with data and labels
                        svmDataDn.push_back(Mat(1, 2, CV_32FC1, x));
                        svmLabelsDn.push_back((int)inputLabels[blkCount]);
                    }
                    blkCount++;
                }
                else
                    rectangle(persImg, extractArea, Scalar(0, 0, 0));
            }
            persImg.copyTo(background(Rect(locPtDn[i], Size(150, 150))));
        }

        imshow("original up", upImg);
        imshow("original down", downImg);
        imshow("trans", background);
    }

    //start training
    cout << "start training svm..\n";
    svmLabelsUp.convertTo(svmLabelsUp, CV_32SC1);
    svmLabelsDn.convertTo(svmLabelsDn, CV_32SC1);
    //up svm
    Ptr<ml::SVM> modelUp = ml::SVM::create();
    modelUp->setType(ml::SVM::C_SVC);
    modelUp->setKernel(ml::SVM::CHI2);
    modelUp->setGamma(5);
    modelUp->setC(10);
    modelUp->setCoef0(0);
    modelUp->setNu(0);
    modelUp->setP(0);
    modelUp->setTermCriteria(TermCriteria(TermCriteria::COUNT + TermCriteria::EPS, 80000, 0.00005));
    //down svm
    Ptr<ml::SVM> modelDn = ml::SVM::create();
    modelDn->setType(ml::SVM::C_SVC);
    modelDn->setKernel(ml::SVM::CHI2);
    modelDn->setGamma(5);
    modelDn->setC(10);
    modelDn->setCoef0(0);
    modelDn->setNu(0);
    modelDn->setP(0);
    modelDn->setTermCriteria(TermCriteria(TermCriteria::COUNT + TermCriteria::EPS, 80000, 0.00005));
    //
    auto tDataUp = ml::TrainData::create(svmDataUp, ml::ROW_SAMPLE, svmLabelsUp);
    auto tDataDn = ml::TrainData::create(svmDataDn, ml::ROW_SAMPLE, svmLabelsDn);
    modelUp->train(tDataUp);
    modelDn->train(tDataDn);
    //
    cout << "training finished..\n";
    modelUp->save(modelPath+"colorParamsUp.xml");
    modelDn->save(modelPath+"colorParamsDn.xml");
}

// Read all six faces colors.
//This is an overload function using SVM to read colors
//The output rank is the same as that defined in class `RubikCube`.
//e.g. "rgbowyyoow....."
void read6faceColor(cv::VideoCapture &downCam, cv::VideoCapture &upCam,
                    char colorLetter[],
                    std::string modelPath)
{
    using namespace clrRecog;

    //check whether model files exist
    auto modelPathUp = modelPath + "colorParamsUp.xml";
    auto modelPathDn = modelPath + "colorParamsDn.xml";
    if (access(modelPathUp.c_str(),F_OK)||access(modelPathDn.c_str(),F_OK))
    {
        printf("color model file(s) do not exist!\n");
        exit(-1);
    }

    //load svm model and init cube.
    Ptr<ml::SVM> modelDn = ml::SVM::load(modelPathDn);
    Ptr<ml::SVM> modelUp = ml::SVM::load(modelPathUp);

    Mat upImg, downImg;
    Mat background(500, 650, CV_8UC3, Scalar(200, 200, 200));
    Mat dspImg(240, 320, CV_8UC3, Scalar(255, 255, 255));

    int initBlkGapX = 7;
    int initBlkGapY = 10;
    int blkGap = 28;
    int blkSize = 25;

    //init cube
    RubikCube cube;
    const char colorName[]{'w', 'r', 'b', 'o', 'g', 'y'};
    char clrLetters[54];
    for (int i = 0; i < 6; i++)
        clrLetters[9 * i + 4] = colorName[i];

    namedWindow("original up", WINDOW_AUTOSIZE);
    namedWindow("original down", WINDOW_AUTOSIZE);
    namedWindow("trans", WINDOW_AUTOSIZE);
    namedWindow("Control", WINDOW_AUTOSIZE);
    //
    cvCreateTrackbar("initBlkGapX", "Control", &initBlkGapX, 15);
    cvCreateTrackbar("initBlkGapY", "Control", &initBlkGapY, 15);
    cvCreateTrackbar("blkGap", "Control", &blkGap, 35);
    cvCreateTrackbar("blkSize", "Control", &blkSize, 30);

    //aquire training samples
    cout << "press 'c' to confirm the result and return\n";
    vector<float> blkResult;
    while (waitKey(30) != 'c')
    {
        upCam >> upImg;
        downCam >> downImg;

        //deal with up camera
        for (int i = 0; i < 3; i++)
        {
            Mat persImg;
            Mat transmtx = getPerspectiveTransform(quadPtSrcUp[i], quadPtDst);
            warpPerspective(upImg, persImg, transmtx, Size(150, 150));

            //extract block color
            for (int j = 0; j < 9; j++)
            {
                if (j == expelBlkNoUp[i] || j == 4)
                    continue;

                int x = j % 3;
                int y = j / 3;
                Rect extractArea((blkGap + blkSize) * x + initBlkGapX, (blkGap + blkSize) * y + initBlkGapY, blkSize, blkSize);

                Mat blkTemp = persImg(extractArea).clone();
                cvtColor(blkTemp, blkTemp, COLOR_BGR2HSV);

                //normalize and predict
                for (int k = 0; k < blkSize * blkSize; k++)
                {
                    float x[2];
                    x[0] = blkTemp.at<Vec3b>(k)[0] / 179.0f;
                    x[1] = blkTemp.at<Vec3b>(k)[1] / 255.0f;
                    Mat predictMat(1, 2, CV_32FC1, x);
                    blkResult.push_back(modelUp->predict(predictMat));
                }

                //count most possible color
                int count[6]{0};
                for (auto lt : blkResult)
                {
                    for (int k = 0; k < 6; k++)
                        if (abs(colorName[k] - lt) < 0.1)
                        {
                            count[k]++;
                            break;
                        }
                }
                blkResult.clear();
                //
                int maxNum = 0, maxIdx = 0;
                for (int k = 0; k < 6; k++)
                    if (count[k] > maxNum)
                    {
                        maxNum = count[k];
                        maxIdx = k;
                    }
                clrLetters[colorLtOffsUp[i] + j] = colorName[maxIdx];

                //draw rectangle of each block
                rectangle(persImg, extractArea, Scalar(0, 0, 0));
            } //end block

            //copy each face's perspective image to background
            persImg.copyTo(background(Rect(locPtUp[i], Size(150, 150))));
        } //end face

        //deal with down camera
        for (int i = 0; i < 3; i++)
        {
            Mat persImg;
            Mat transmtx = getPerspectiveTransform(quadPtSrcDn[i], quadPtDst);
            warpPerspective(downImg, persImg, transmtx, Size(150, 150));

            //extract block color
            for (int j = 0; j < 9; j++)
            {
                if (j == expelBlkNoDn[i] || j == 4)
                    continue;

                int x = j % 3;
                int y = j / 3;
                Rect extractArea((blkGap + blkSize) * x + initBlkGapX, (blkGap + blkSize) * y + initBlkGapY, blkSize, blkSize);

                Mat blkTemp = persImg(extractArea).clone();
                cvtColor(blkTemp, blkTemp, COLOR_BGR2HSV);

                //normalize and predict
                for (int k = 0; k < blkSize * blkSize; k++)
                {
                    float x[2];
                    x[0] = blkTemp.at<Vec3b>(k)[0] / 179.0f;
                    x[1] = blkTemp.at<Vec3b>(k)[1] / 255.0f;
                    Mat predictMat(1, 2, CV_32FC1, x);
                    blkResult.push_back(modelDn->predict(predictMat));
                }

                //count most possible color
                int count[6]{0};
                for (auto lt : blkResult)
                {
                    for (int k = 0; k < 6; k++)
                        if (abs(colorName[k] - lt) < 0.1)
                        {
                            count[k]++;
                            break;
                        }
                }
                blkResult.clear();
                //
                int maxNum = 0, maxIdx = 0;
                for (int k = 0; k < 6; k++)
                    if (count[k] > maxNum)
                    {
                        maxNum = count[k];
                        maxIdx = k;
                    }
                clrLetters[colorLtOffsDn[i] + j] = colorName[maxIdx];

                //draw rectangle of each block
                rectangle(persImg, extractArea, Scalar(0, 0, 0));
            } //end block

            //copy each face's perspective image to background
            persImg.copyTo(background(Rect(locPtDn[i], Size(150, 150))));
        } //end face

        inferUnknownClr(clrLetters);
        cube.initCubeState(clrLetters);
        cube.display(dspImg, 10, 10);

        imshow("original up", upImg);
        imshow("original down", downImg);
        imshow("trans", background);
        imshow("display", dspImg);
    }//end while(waitKey(30))

    //copy result to return array
    for (int i = 0; i < 54;i++)
        colorLetter[i] = clrLetters[i];
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
