#include "RubikCube.h"
#include "colorRecog.h"

using namespace std;
using namespace cv;

// display the results
static void *display_thread(void *data);

Mat upImg, downImg;
Mat background(500, 650, CV_8UC3, Scalar(200, 200, 200));
Mat dspImg(240, 320, CV_8UC3, Scalar(255, 255, 255));
char key = '\0';

int main(int argc, char **argv)
{
    int choice = 1;
    cout << "enter one of the following numbers:\n"
            "1. record color samples and train models\n"
            "2. recognize colors of six faces and display them\n"
            "Please enter your choice: ";
    cin >> choice;

    if (choice == 1)
    {
        int camIdx[2];
        getOrderCamera(camIdx);
        //
        VideoCapture downCam(camIdx[0]);
        VideoCapture upCam(camIdx[1]);

        trainColorModel(downCam, upCam);
    }
    else
    {
        char colorLetters[55];
        colorLetters[54] = '\0';

        // create display thread
        pthread_t displayThread;
        pthread_create(&displayThread, NULL, display_thread, NULL);

        RubikCube cube;

        cout << "'r' to read color. 'q' to quit.\n\n";
        while (1)
        {
            if (key == 'q')
                break;
            else if (key != 'r')
                continue;

            read6faceColor(downImg.clone(), upImg.clone(), background, colorLetters);

            cube.initCubeState(colorLetters);
            cube.display(dspImg, 10, 10);

            printf("The color array is:\n%s\n", colorLetters);
        }
    }
}

// display thread
static void *display_thread(void *data)
{
    int camIdx[2];
    getOrderCamera(camIdx);
    //
    VideoCapture downCam(camIdx[0]);
    VideoCapture upCam(camIdx[1]);

    while (key != 'q')
    {
        upCam >> upImg;
        downCam >> downImg;

        imshow("original up", upImg);
        imshow("original down", downImg);
        imshow("trans", background);
        imshow("display", dspImg);
        key = waitKey(100);
    }

    pthread_exit(0);
}