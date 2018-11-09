// #include <unistd.h>
#include <vector>
#include "RubikCube.h"
#include "control.h"

using namespace std;

//convert "U L' F2 ..." to 048....
static void sol2turnMethodNum(const char *sol, vector<int> &turnMethodNum);

//find solution by lib kociemba
//U R F D L B order in facelets
extern "C"
{
    char *solution(char *facelets, int maxDepth,
                   long timeOut, int useSeparator,
                   const char *cache_dir);
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        cout << "you must tell me the colors!" << endl;
        cout << "Input should be like this: yyybrgowwb..\n";
        cout << "The rank of faces are ULFRBD";
        return -1;
    }

    RubikCube cube;
    cube.initCubeState(argv[1]);

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

//convert "U L' F2 ..." to 048....
static void sol2turnMethodNum(const char *const sol, vector<int> &turnMethodNum)
{
    int num;
    for (const char *p = sol; *p != '\0'; p++)
    {
        switch (*p)
        {
        case 'U':
            num = 0;
            break;
        case 'L':
            num = 3;
            break;
        case 'F':
            num = 6;
            break;
        case 'R':
            num = 9;
            break;
        case 'B':
            num = 12;
            break;
        case 'D':
            num = 15;
            break;
        }

        //judge the character following the face note
        p++;
        if (*p == '\'')
        {
            num += 1;
            p++;
        }
        else if (*p == '2')
        {
            num += 2;
            p++;
        }

        turnMethodNum.push_back(num);
    }
}
