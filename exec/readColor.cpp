#include "RubikCube.h"
#include "colorRecog.h"

using namespace std;
using namespace cv;

int main(int argc, char **argv)
{
    int choice = 1;
    cout << "enter one of the following numbers:\n"
            "1. record color samples and train models\n"
            "2. recognize colors of six faces and display them\n"
            "Please enter your choice: ";
    cin >> choice;

    int camIdx[2];
    getOrderCamera(camIdx);
    //
    VideoCapture downCam(camIdx[0]);
    VideoCapture upCam(camIdx[1]);

    if(choice==1)
        trainColorModel(downCam, upCam);
    else
    {
        char colorLetters[54];
        read6faceColor(downCam, upCam, colorLetters);
    }
}
