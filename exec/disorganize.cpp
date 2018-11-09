#include <vector>
#include <iostream>
#include "RubikCube.h"
#include "control.h"

using namespace std;

int main(int argc, char **argv)
{
    cout << "This program disorganize the cube" << endl;

    vector<int> steps;
    genRandomState(steps);

    //drive the motors to solve it
    cout << "random state generated. Start to disorganize..\n";
    Motor sixMotors(100, 100);
    sixMotors.drive6motor(steps);

    //end of program
    return 0;
}
