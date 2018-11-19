// #include <unistd.h>
#include <vector>
#include "RubikCube.h"
#include "colorRecog.h"
#include "Motor.h"

using namespace std;
using namespace cv;

//find solution by lib kociemba
//U R F D L B order in facelets
extern "C"
{
    char *solution(char *facelets, int maxDepth,
                   long timeOut, int useSeparator,
                   const char *cache_dir);
}

//display the results
static void *display_thread(void *data);

Mat upImg, downImg;
Mat background(500, 650, CV_8UC3, Scalar(200, 200, 200));
Mat dspImg(240, 320, CV_8UC3, Scalar(255, 255, 255));
char key = '\0';

int main(int argc, char **argv)
{
    int choice = 1;
    cout << "Solve cube : 1. automatically; 2. manually\n"
            "Please enter your choice: ";
    cin >> choice;

    char colorLetters[55];
    colorLetters[54] = '\0';

    if (choice == 1) //auto
    {
        //create display thread
        pthread_t displayThread;
        pthread_create(&displayThread, NULL, display_thread, NULL);

        RubikCube cube;

        cout << "press 'q' to quit. 'r' to read color. 'c' to confirm result.\n";
        while (1)
        {
            if (key == 'c')
                break;
            else if (key != 'r')
                continue;

            read6faceColor(downImg.clone(), upImg.clone(), background, colorLetters);

            cube.initCubeState(colorLetters);
            cube.display(dspImg, 10, 10);

            printf("The color array is:\n%s\n", colorLetters);
        }
    }
    else //manually
    {
        cout << "Please tell me the colors: ";
        cin >> colorLetters;
    }

    RubikCube cube;
    cube.initCubeState(colorLetters);

    //find cube solution
    //NOTE: The face order is different, you need to convert first.
    char facelets[55];
    int const offset[6]{0, 18, 0, 18, -27, -9};
    for (int i = 0; i < 54; i++)
    {
        facelets[i] = cube.orientData[i + offset[i / 9]];
    }
    facelets[54] = '\0';
    char *sol = solution(facelets, 24, 1000, 0, "cache");

    //deal with unsolvable situation
    if (NULL == sol)
    {
        printf("Errors occur! Unsolvable cube! Please check you input.\n");
        return -1;
    }

    //convert sol to turnMethod number
    //i.e. "U L' F2 ..." to 048....
    vector<int> turnMethodNum;
    sol2turnMethodNum(sol, turnMethodNum);
    puts(sol);
    free(sol);

    //drive the motors to solve it
    Motor sixMotors(100, 100);
    sixMotors.drive6motor(turnMethodNum);

    //end of program
    return 0;
}

//display thread
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