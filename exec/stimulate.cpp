#include <opencv2/opencv.hpp>
#include <pthread.h>
#include "RubikCube.h"

using namespace std;
using namespace cv;

//display the results
static void *display_thread(void *data);

int main()
{
    Face::COLOR color[]{Face::YELLOW, Face::WHITE, Face::BLUE, 
                        Face::YELLOW, Face::BLUE, Face::RED, 
                        Face::ORANGE, Face::GREEN, Face::WHITE, 
                        Face::GREEN, Face::BLUE, Face::GREEN, 
                        Face::ORANGE, Face::ORANGE, Face::WHITE, 
                        Face::BLUE, Face::WHITE, Face::ORANGE,
                        Face::WHITE, Face::ORANGE, Face::RED,
                        Face::GREEN, Face::WHITE, Face::GREEN,
                        Face::GREEN, Face::BLUE, Face::YELLOW,
                        Face::BLUE, Face::YELLOW, Face::ORANGE, 
                        Face::RED, Face::RED, Face::ORANGE, 
                        Face::BLUE, Face::RED, Face::RED, 
                        Face::WHITE, Face::BLUE, Face::RED,
                        Face::WHITE, Face::YELLOW, Face::YELLOW, 
                        Face::GREEN, Face::GREEN, Face::RED,
                        Face::YELLOW, Face::ORANGE, Face::ORANGE, 
                        Face::RED, Face::GREEN, Face::BLUE, 
                        Face::YELLOW, Face::YELLOW, Face::WHITE};

    RubikCube cube;
    cube.setAllFaceColor(color);

    //create display thread
    pthread_t displayThread;
    pthread_create(&displayThread, NULL, display_thread, &cube);

    //call turning method
    int i = 0;
    while(1)
    {
        cin >> i;
        if(i==-1)
            break;
        (cube.*(cube.turnMethod)[i])();
    }

    pthread_join(displayThread, NULL);
    return 0;
}

//display thread
static void *display_thread(void *data)
{
    auto &cube = *(RubikCube *)data;
    Mat img(480, 640, CV_8UC3, Scalar(255, 255, 255));
    namedWindow("show", WINDOW_AUTOSIZE);

    while ((char)waitKey(300) != 'q')
    {
        cube.display(img, 100, 200);
        imshow("show", img);
    }

    pthread_exit(0);
}