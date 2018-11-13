#include "RubikCube.h"
#include "colorRecog.h"

using namespace std;
using namespace cv;

int main(int argc, char **argv)
{
    int camIdx[2];
    getOrderCamera(camIdx);

    VideoCapture downCam(camIdx[0]);
    VideoCapture upCam(camIdx[1]);
    // Mat upImage = imread("../data/up2.jpg");
    // Mat downImage = imread("../data/down2.jpg");

    Mat upImage, downImage;
    RubikCube cube;
    namedWindow("show", WINDOW_AUTOSIZE);
    
    while (waitKey(30) != 'q')
    {
        upCam >> upImage;
        downCam >> downImage;

        char colorLetter[54];
        read6faceColor(upImage, downImage, colorLetter);

        cube.initCubeState(colorLetter);
        Mat dspImg(240, 320, CV_8UC3, Scalar(255, 255, 255));
        cube.display(dspImg, 10, 10);
        imshow("show", dspImg);
    }

    return 0;
}
