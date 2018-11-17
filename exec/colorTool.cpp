#include "colorRecog.h"
#include "RubikCube.h"
#include <opencv2/opencv.hpp>
#include <opencv2/ml/ml.hpp>

using namespace std;
using namespace cv;
// using namespace ml;

static void onMouseHandle(int event, int x, int y, int flags, void *param);
static void take_photo(Mat &img);
static void adjHsvValue();          //tool 1
static void kmean_test();           //tool 2
static void perspective();          //tool 3
static void pers_kmeans();          //tool 4
static void svm_train_test();       //tool 5
static void svm_predict_test();     //tool 6

//-------------------------------------------------------------
int main(int argc, char **argv)
{
    cout << "enter number to start specific function:\n"
            "1. adjust hsv value\n"
            "2. kmeans test\n"
            "3. perspective test\n"
            "4. perspective and kmeans together\n"
            "5. svm color train test\n"
            "6. svm color predict test\n";

    int choice = 1;
    cin >> choice;
    switch (choice)
    {
    case 1:
        adjHsvValue();
        break;
    case 2:
        kmean_test();
        break;
    case 3:
        perspective();
        break;
    case 4:
        pers_kmeans();
        break;
    case 5:
        svm_train_test();
        break;
    case 6:
        svm_predict_test();
        break;
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

static void kmean_test()
{
    Mat data, labels, srcHsv;
    Mat srcImg = imread("/home/savage/Downloads/71a37b2f1c421526eaae3e4edea9ddd4.jpg");
    cvtColor(srcImg, srcHsv, COLOR_BGR2HSV);
    srcImg.reshape(0, srcImg.rows * srcImg.cols).convertTo(data, CV_32FC3);

    int K = 3, attempt = 10;
    kmeans(data, K, labels,
           TermCriteria(TermCriteria::COUNT + TermCriteria::EPS, 500, 0.001), attempt,
           KMEANS_PP_CENTERS);

    Mat lb1 = srcImg.clone();
    Mat lb2 = srcImg.clone();
    Mat lb3 = srcImg.clone();

    for (int i = 0; i < srcImg.rows * srcImg.cols; i++)
    {
        if (labels.at<int>(i) == 0)
        {
            lb2.at<Vec3b>(i) = Vec3b(255, 255, 255);
            lb3.at<Vec3b>(i) = Vec3b(255, 255, 255);
        }
        else if (labels.at<int>(i) == 1)
        {
            lb1.at<Vec3b>(i) = Vec3b(255, 255, 255);
            lb3.at<Vec3b>(i) = Vec3b(255, 255, 255);
        }
        else
        {
            lb1.at<Vec3b>(i) = Vec3b(255, 255, 255);
            lb2.at<Vec3b>(i) = Vec3b(255, 255, 255);
        }
    }

    imshow("label1", lb1);
    imshow("label2", lb2);
    imshow("label3", lb3);
    waitKey(0);
}

//perspective
static void perspective()
{
    //get video objects
    int camIdx[2];
    getOrderCamera(camIdx);
    //
    VideoCapture upCam(camIdx[1]);
    Mat upImg;
    Mat background(400, 500, CV_8UC3, Scalar(200, 200, 200));

    //rank is 'up, right, back'
    const vector<vector<Point2f>> quadPtSrc{
        vector<Point2f>{Point2f(564, 249), Point2f(363, 188), Point2f(305, 56), Point2f(482, 116)},
        vector<Point2f>{Point2f(299, 58), Point2f(355, 192), Point2f(241, 377), Point2f(205, 224)},
        vector<Point2f>{Point2f(368, 198), Point2f(558, 261), Point2f(437, 416), Point2f(248, 380)}};
    const vector<Point2f> quadPtDst{Point2f(0, 0), Point2f(100, 0), Point2f(100, 100), Point2f(0, 100)};
    //
    const vector<Point> locPt{Point(130, 20), Point(240, 130), Point(350, 130)};
    //
    const int expelBlkNo[]{6, 6, 8};    //start idx is 0
    int blkSize = 15;
    int blkGap = 22;
    int initBlkGapX = 5;
    int initBlkGapY = 5;

    namedWindow("ori", WINDOW_AUTOSIZE);
    namedWindow("trans", WINDOW_AUTOSIZE);
    namedWindow("Control", WINDOW_AUTOSIZE);
    //
    cvCreateTrackbar("initBlkGapX", "Control", &initBlkGapX, 10);
    cvCreateTrackbar("initBlkGapY", "Control", &initBlkGapY, 10);
    cvCreateTrackbar("blkGap", "Control", &blkGap, 35);
    cvCreateTrackbar("blkSize", "Control", &blkSize, 25);
    //
    setMouseCallback("ori", onMouseHandle, nullptr);
    setMouseCallback("trans", onMouseHandle, nullptr);

    while (waitKey(30) != 'q')
    {
        upCam >> upImg;

        for (int i = 0; i < 3;i++)
        {
            Mat persImg;
            Mat transmtx = getPerspectiveTransform(quadPtSrc[i], quadPtDst);
            warpPerspective(upImg, persImg, transmtx, Size(100, 100));

            //draw rectangles
            for (int j = 0; j < 9;j++)
            {
                if (j == expelBlkNo[i] || j == 4)
                    continue;

                int x = j % 3;
                int y = j / 3;
                Rect extractArea((blkGap + blkSize) * x + initBlkGapX, (blkGap + blkSize) * y + initBlkGapY, blkSize, blkSize);
                rectangle(persImg, extractArea, Scalar(0, 0, 0));
            }

            persImg.copyTo(background(Rect(locPt[i], Size(100, 100))));
        }
        
        imshow("ori", upImg);
        imshow("trans", background);
    }
}

//perspective and Kmeans
static void pers_kmeans()
{
    //get video objects
    int camIdx[2];
    getOrderCamera(camIdx);
    //
    VideoCapture upCam(camIdx[1]);
    Mat upImg;
    Mat background(500, 650, CV_8UC3, Scalar(200, 200, 200));

    //rank is 'up, right, back'
    const vector<vector<Point2f>> quadPtSrc{
        vector<Point2f>{Point2f(564, 249), Point2f(363, 188), Point2f(305, 56), Point2f(482, 116)},
        vector<Point2f>{Point2f(299, 58), Point2f(355, 192), Point2f(241, 377), Point2f(205, 224)},
        vector<Point2f>{Point2f(368, 198), Point2f(558, 261), Point2f(437, 416), Point2f(248, 380)}};
    const vector<Point2f> quadPtDst{Point2f(0, 0), Point2f(150, 0), Point2f(150, 150), Point2f(0, 150)};
    //
    const vector<Point> locPt{Point(170, 10), Point(330, 170), Point(490, 170)};
    //
    const int expelBlkNo[]{6, 6, 8}; //start idx is 0
    int initBlkGapX = 7;
    int initBlkGapY = 10;
    int blkGap = 28;
    int blkSize = 25;

    namedWindow("ori", WINDOW_AUTOSIZE);
    namedWindow("trans", WINDOW_AUTOSIZE);
    namedWindow("Control", WINDOW_AUTOSIZE);
    //
    cvCreateTrackbar("initBlkGapX", "Control", &initBlkGapX, 15);
    cvCreateTrackbar("initBlkGapY", "Control", &initBlkGapY, 15);
    cvCreateTrackbar("blkGap", "Control", &blkGap, 35);
    cvCreateTrackbar("blkSize", "Control", &blkSize, 30);
    //
    setMouseCallback("ori", onMouseHandle, nullptr);
    setMouseCallback("trans", onMouseHandle, nullptr);

    //preparation for Kmeans
    Mat kmeanData, labels, outCenter;
    int K = 6, attempt = 10;

    //aquire training samples
    cout << "press 't' to take train samples, 'n' to start training..\n";
    while(1)
    {
        upCam >> upImg;
        char key = waitKey(30);
        if (key == 'n')
            break;
        else if (key == 't')
            cout << "sample recorded\n";

        for (int i = 0; i < 3; i++)
        {
            Mat persImg;
            Mat transmtx = getPerspectiveTransform(quadPtSrc[i], quadPtDst);
            warpPerspective(upImg, persImg, transmtx, Size(150, 150));

            //extract block color
            for (int j = 0; j < 9; j++)
            {
                if (j == expelBlkNo[i] || j == 4)
                    continue;
                
                int x = j % 3;
                int y = j / 3;
                Rect extractArea((blkGap + blkSize) * x + initBlkGapX, (blkGap + blkSize) * y + initBlkGapY, blkSize, blkSize);
                
                if(key=='t')
                {
                    Mat blkTemp = persImg(extractArea).clone();
                    cvtColor(blkTemp, blkTemp, COLOR_BGR2HSV);
                    blkTemp.reshape(0, blkSize * blkSize).convertTo(blkTemp, CV_32FC3);
                    
                    //normalize
                    for (int k = 0; k < blkSize * blkSize;k++)
                    {
                        blkTemp.at<Vec3f>(k)[0] /= 179.0f;
                        blkTemp.at<Vec3f>(k)[1] /= 255.0f;
                        // blkTemp.at<Vec3f>(k)[2] /= 255.0f;
                    }

                    //discard V channel
                    Mat channels[3];
                    split(blkTemp, channels);
                    merge(channels, 2, blkTemp);
                    
                    kmeanData.push_back(blkTemp);
                }
                else
                    rectangle(persImg, extractArea, Scalar(0, 0, 0));
            }
            persImg.copyTo(background(Rect(locPt[i], Size(150, 150))));
        }

        imshow("ori", upImg);
        imshow("trans", background);
    }

    //start training
    cout << "start training Kmeans..\n";
    kmeans(kmeanData, K, labels,
           TermCriteria(TermCriteria::COUNT + TermCriteria::EPS, 500, 0.001), attempt,
           KMEANS_PP_CENTERS, outCenter);
    cout << "training finished..\n";

    //display outCenter colors
    background(Range::all(), Range::all()) = Vec3b(200, 200, 200);
    Mat distribution(180, 256, CV_8UC3, Scalar(50, 0, 255));
    for (int i = 0; i < 6; i++)
    {
        Mat colorVals;

        //denomalize
        outCenter.at<Vec2f>(i)[0] *= 179;
        outCenter.at<Vec2f>(i)[1] *= 255;
        // outCenter.at<Vec3f>(i)[2] *= 255;

        outCenter.row(i).convertTo(colorVals, CV_8UC2);
        Scalar color(colorVals.at<uchar>(0),
                     colorVals.at<uchar>(1),
                     180);
        
        Mat colorTemp(50, 50, CV_8UC3, color);
        cvtColor(colorTemp, colorTemp, COLOR_HSV2BGR);
        colorTemp.copyTo(background(Rect(50 * i, 100, 50, 50)));
    }
    //
    //draw distribution
    for (int i = 0; i < kmeanData.cols * kmeanData.rows; i++)
    {
        int label = labels.at<int>(i);
        Scalar color(outCenter.at<Vec2f>(label)[0],
                     outCenter.at<Vec2f>(label)[1],
                     180);

        circle(distribution,
               Point(kmeanData.at<Vec2f>(i)[1] * 255, kmeanData.at<Vec2f>(i)[0] * 179),
               1,
               color,
               -1);
    }
    cvtColor(distribution, distribution, COLOR_HSV2BGR);

    imshow("distribution", distribution);
    imshow("center output", background);
    waitKey(0);
}

static void svm_train_test()
{
    //get video objects
    int camIdx[2];
    getOrderCamera(camIdx);
    //
    VideoCapture upCam(camIdx[1]);
    Mat upImg;
    Mat background(500, 650, CV_8UC3, Scalar(200, 200, 200));

    //rank is 'up, right, back'
    const vector<vector<Point2f>> quadPtSrc{
        vector<Point2f>{Point2f(564, 249), Point2f(363, 188), Point2f(305, 56), Point2f(482, 116)},
        vector<Point2f>{Point2f(299, 58), Point2f(355, 192), Point2f(241, 377), Point2f(205, 224)},
        vector<Point2f>{Point2f(368, 198), Point2f(558, 261), Point2f(437, 416), Point2f(248, 380)}};
    const vector<Point2f> quadPtDst{Point2f(0, 0), Point2f(150, 0), Point2f(150, 150), Point2f(0, 150)};
    //
    const vector<Point> locPt{Point(170, 10), Point(330, 170), Point(490, 170)};
    //
    const int expelBlkNo[]{6, 6, 8}; //start idx is 0
    int initBlkGapX = 7;
    int initBlkGapY = 10;
    int blkGap = 28;
    int blkSize = 25;

    namedWindow("ori", WINDOW_AUTOSIZE);
    namedWindow("trans", WINDOW_AUTOSIZE);
    namedWindow("Control", WINDOW_AUTOSIZE);
    //
    cvCreateTrackbar("initBlkGapX", "Control", &initBlkGapX, 15);
    cvCreateTrackbar("initBlkGapY", "Control", &initBlkGapY, 15);
    cvCreateTrackbar("blkGap", "Control", &blkGap, 35);
    cvCreateTrackbar("blkSize", "Control", &blkSize, 30);
    //
    setMouseCallback("ori", onMouseHandle, nullptr);
    setMouseCallback("trans", onMouseHandle, nullptr);

    //preparation for SVM
    Mat svmData, svmLabels;
    char inputLabels[60];

    //aquire training samples
    cout << "press 't' to take train samples, 'n' to start training..\n";
    while (1)
    {
        upCam >> upImg;
        char key = waitKey(30);
        if (key == 'n')
            break;
        else if (key == 't')
        {
            cout << "please enter the color numbers: ";
            cin >> inputLabels;
            cout << "sample recorded.\n\n";
        }

        int blkCount = 0;
        for (int i = 0; i < 3; i++)
        {
            Mat persImg;
            Mat transmtx = getPerspectiveTransform(quadPtSrc[i], quadPtDst);
            warpPerspective(upImg, persImg, transmtx, Size(150, 150));

            //extract block color
            for (int j = 0; j < 9; j++)
            {
                if (j == expelBlkNo[i] || j == 4)
                    continue;

                int x = j % 3;
                int y = j / 3;
                Rect extractArea((blkGap + blkSize) * x + initBlkGapX, (blkGap + blkSize) * y + initBlkGapY, blkSize, blkSize);

                if (key == 't')
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
                        svmData.push_back(Mat(1, 2, CV_32FC1, x));
                        svmLabels.push_back((int)inputLabels[blkCount]);
                    }
                    blkCount++;
                }
                else
                    rectangle(persImg, extractArea, Scalar(0, 0, 0));
            }
            persImg.copyTo(background(Rect(locPt[i], Size(150, 150))));
        }

        imshow("ori", upImg);
        imshow("trans", background);
    }

    //start training
    cout << "start training svm..\n";
    svmLabels.convertTo(svmLabels, CV_32SC1);
    //
    Ptr<ml::SVM> model = ml::SVM::create();
    model->setType(ml::SVM::C_SVC);
    model->setKernel(ml::SVM::CHI2);
    model->setGamma(5);
    model->setC(10);
    model->setCoef0(0);
    model->setNu(0);
    model->setP(0);
    model->setTermCriteria(TermCriteria(TermCriteria::COUNT + TermCriteria::EPS, 80000, 0.00005));
    //
    auto tData = ml::TrainData::create(svmData, ml::ROW_SAMPLE, svmLabels);
    model->train(tData);
    //
    cout << "training finished..\n";
    model->save("./colorParams.xml");
}

// static void svm_predict_test()
// {
//     float te[2]{100.0 / 179.0, 160.0 / 255.0};
//     Mat a(1, 2, CV_32FC1, te);

//     Ptr<ml::SVM> model = ml::SVM::load("./colorParams.xml");
//     char ou = (char)model->predict(a);
//     cout << "predict is " << ou << endl;
// }


static void svm_predict_test()
{
    //get video objects
    int camIdx[2];
    getOrderCamera(camIdx);
    //
    VideoCapture upCam(camIdx[1]);
    Mat upImg;
    Mat background(500, 650, CV_8UC3, Scalar(200, 200, 200));
    Mat dspImg(240, 320, CV_8UC3, Scalar(255, 255, 255));

    //load svm model and init cube.
    Ptr<ml::SVM> model = ml::SVM::load("./colorParams.xml");
    
    //init cube
    RubikCube cube;
    char colorLetters[54];
    for (auto& x:colorLetters)
        x = 'w';
    colorLetters[31] = 'o';
    colorLetters[40] = 'g';

    //rank is 'up, right, back'
    const vector<vector<Point2f>> quadPtSrc{
        vector<Point2f>{Point2f(564, 249), Point2f(363, 188), Point2f(305, 56), Point2f(482, 116)},
        vector<Point2f>{Point2f(299, 58), Point2f(355, 192), Point2f(241, 377), Point2f(205, 224)},
        vector<Point2f>{Point2f(368, 198), Point2f(558, 261), Point2f(437, 416), Point2f(248, 380)}};
    const vector<Point2f> quadPtDst{Point2f(0, 0), Point2f(150, 0), Point2f(150, 150), Point2f(0, 150)};
    //
    const vector<Point> locPt{Point(170, 10), Point(330, 170), Point(490, 170)};
    const int colorLtOffset[]{0, 27, 36};
    const char colorName[]{'w', 'r', 'b', 'o', 'g', 'y'};
    //
    const int expelBlkNo[]{6, 6, 8}; //start idx is 0
    int initBlkGapX = 7;
    int initBlkGapY = 10;
    int blkGap = 28;
    int blkSize = 25;

    namedWindow("ori", WINDOW_AUTOSIZE);
    namedWindow("trans", WINDOW_AUTOSIZE);
    namedWindow("Control", WINDOW_AUTOSIZE);
    //
    cvCreateTrackbar("initBlkGapX", "Control", &initBlkGapX, 15);
    cvCreateTrackbar("initBlkGapY", "Control", &initBlkGapY, 15);
    cvCreateTrackbar("blkGap", "Control", &blkGap, 35);
    cvCreateTrackbar("blkSize", "Control", &blkSize, 30);
    //
    setMouseCallback("ori", onMouseHandle, nullptr);
    setMouseCallback("trans", onMouseHandle, nullptr);

    //aquire training samples
    cout <<"press 'q' to quit\n";
    vector<float> blkResult;
    while (waitKey(30)!='q')
    {
        upCam >> upImg;

        for (int i = 0; i < 3; i++)
        {
            Mat persImg;
            Mat transmtx = getPerspectiveTransform(quadPtSrc[i], quadPtDst);
            warpPerspective(upImg, persImg, transmtx, Size(150, 150));

            //extract block color
            for (int j = 0; j < 9; j++)
            {
                if (j == expelBlkNo[i] || j == 4)
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
                    blkResult.push_back(model->predict(predictMat));
                }

                //count most possible color
                int count[6]{0};
                for (auto lt : blkResult)
                {
                    for (int k = 0; k < 6;k++)
                    if(abs(colorName[k]-lt)<0.1)
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
                colorLetters[colorLtOffset[i] + j] = colorName[maxIdx];

                //draw rectangle of each block
                rectangle(persImg, extractArea, Scalar(0, 0, 0));
            }//end block

            //copy each face's perspective image to background
            persImg.copyTo(background(Rect(locPt[i], Size(150, 150))));
        }//end face

        cube.initCubeState(colorLetters);
        cube.display(dspImg, 10, 10);

        imshow("ori", upImg);
        imshow("trans", background);
        imshow("display", dspImg);
    }
}
