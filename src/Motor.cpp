/* ********************************************************
*   This is the realization of header control.h

* @Author : Derek Lai
* @Date   : 2018/11/7
* *******************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "Motor.h"
#include "wiringPi.h"

using namespace std;

//---------------------begin class `Motor` ---------------------------------
//public:
const int Motor::MOTOR_PINS[]{1, 3, 5, 7, 22, 24};
const int Motor::DIR_PINS[]  {0, 2, 4, 6, 21, 23};

const int Motor::MT_CCW = 0;
const int Motor::MT_CW = 1;

//construct function. Init 6 motors(12 pins)
Motor::Motor(unsigned int peri, unsigned int delayT):
    period(peri),delayTime(delayT)
{
    init6motor();
}

//drive 6 motors according to an array `steps`
//e.g. `steps` = 0 4 8 11 1 3 .....
//acutual movement is U_90, L90, F180.....
//see more relation at file 'RubikCube.h'
void Motor::drive6motor(const std::vector<int> &steps) const
{
    const int oppMotorList[]{5, 3, 4, 1, 2, 0};
    int face, faceNext = -1;  //0U 1L 2F 3R 4B 5D
                       //degree 0(-90)  1(90)  2(180)

    //analyze steps
    vector<int> face1, face2;
    vector<int> degree1, degree2;
    vector<int> direction1, direction2;
    //
    for (size_t i = 0; i < steps.size() - 1; i++)
    {
        face = steps[i] / 3;
        faceNext = steps[i + 1] / 3;

        if(oppMotorList[face] == faceNext)
        {
            face1.push_back(face);
            face2.push_back(faceNext);

            //0(-90)  1(90)  2(180)
            //1
            if(steps[i] % 3 == 0)
            {
                degree1.push_back(90);
                direction1.push_back(MT_CW);
            }
            else if(steps[i] % 3 == 1)
            {
                degree1.push_back(90);
                direction1.push_back(MT_CCW);
            }
            else
            {
                degree1.push_back(180);
                direction1.push_back(MT_CCW);
            }
            //2
            if (steps[i + 1] % 3 == 0)
            {
                degree2.push_back(90);
                direction2.push_back(MT_CW);
            }
            else if (steps[i + 1] % 3 == 1)
            {
                degree2.push_back(90);
                direction2.push_back(MT_CCW);
            }
            else
            {
                degree2.push_back(180);
                direction2.push_back(MT_CCW);
            }

            i++;
            continue;
        }
        else
        {
            face1.push_back(face);
            face2.push_back(-1);

            //0(-90)  1(90)  2(180)
            //1
            if (steps[i] % 3 == 0)
            {
                degree1.push_back(90);
                direction1.push_back(MT_CW);
            }
            else if (steps[i] % 3 == 1)
            {
                degree1.push_back(90);
                direction1.push_back(MT_CCW);
            }
            else
            {
                degree1.push_back(180);
                direction1.push_back(MT_CCW);
            }

            degree2.push_back(-1);
            direction2.push_back(-1);

            //deal with the last two steps.
            if (i != steps.size() - 2)
                continue;

            face1.push_back(faceNext);
            face2.push_back(-1);

            //0(-90)  1(90)  2(180)
            //1
            if (steps[i + 1] % 3 == 0)
            {
                degree1.push_back(90);
                direction1.push_back(MT_CW);
            }
            else if (steps[i + 1] % 3 == 1)
            {
                degree1.push_back(90);
                direction1.push_back(MT_CCW);
            }
            else
            {
                degree1.push_back(180);
                direction1.push_back(MT_CCW);
            }

            degree2.push_back(-1);
            direction2.push_back(-1);
            break;
        }
    }

    //turn the motors
    for (size_t i = 0; i < face1.size(); i++)
    {
        if(face2[i]!=-1)    //turn together
        {
            changeDirection(DIR_PINS[face1[i]], direction1[i]);
            changeDirection(DIR_PINS[face2[i]], direction2[i]);

            driveOppMotors(MOTOR_PINS[face1[i]], MOTOR_PINS[face2[i]],
                           degree1[i], degree2[i]);
        }
        else    //turn single motor
        {
            changeDirection(DIR_PINS[face1[i]], direction1[i]);
            driveSingleMotor(MOTOR_PINS[face1[i]], degree1[i]);
        }
    }
}
//private:
//change motor direction
void Motor::changeDirection(int dirPin, int direction)const
{
    digitalWrite(dirPin, direction);
    delayMicroseconds(50);
}

//drive two opposite motors. For optimization usage.
void Motor::driveOppMotors(int motorPin1, int motorPin2,
                           int degree1,   int degree2) const
{
    //steps determine the turning degree
    int steps1 = 0, steps2 = 0;
    
    if(degree1 == 90)
        steps1 = 200;
    else
        steps1 = 400;

    if (degree2 == 90)
        steps2 = 200;
    else
        steps2 = 400;
    
    //calculate smaller steps to turn together
    int steps = steps1 <= steps2 ? steps1 : steps2; 
    int remainStep = steps1 - steps2;

    //turn smaller steps together
    for (int i = 0; i < steps; i++)
    {
        digitalWrite(motorPin1, HIGH);
        digitalWrite(motorPin2, HIGH);
        delayMicroseconds(period);
        digitalWrite(motorPin1, LOW);
        digitalWrite(motorPin2, LOW);
        delayMicroseconds(period);
    }

    //turn remain steps
    if(remainStep > 0)
    {
        for (int i = 0; i < 200; i++)
        {
            digitalWrite(motorPin1, HIGH);
            delayMicroseconds(period);
            digitalWrite(motorPin1, LOW);
            delayMicroseconds(period);
        }
    }
    else if(remainStep < 0)
    {
        for (int i = 0; i < 200; i++)
        {
            digitalWrite(motorPin2, HIGH);
            delayMicroseconds(period);
            digitalWrite(motorPin2, LOW);
            delayMicroseconds(period);
        }
    }

    delay(delayTime);
}

//drive the motor specify by `motorNum` to turn `degree`
//NOTE: the speed(period and delayTime) is set to fix values.
void Motor::driveSingleMotor(int motorPin, int degree)const
{
    //steps determine the turning degree
    int steps = 0;
    if (degree == 90)
        steps = 200;
    else
        steps = 400;
    
    //start to turn
    for (int i = 0; i < steps; i++)
    {
        digitalWrite(motorPin, HIGH);
        delayMicroseconds(period);
        digitalWrite(motorPin, LOW);
        delayMicroseconds(period);
    }
    delay(delayTime);
}

//get 12 pins ready for output.
//Wiring Pi: 0  1  | 2  3  | 4  5  | 6  7 | 21 22 | 23 24
//board pin: 11 12 | 13 15 | 16 18 | 22 7 | 29 31 | 33 35
//motor    :   U   |   L   |   F   |   R  |   B   |   D
//dir+     : 1     | 1     | 1     | 1    | 1     | 1
//pul+     :     1 |     1 |     1 |    1 |     1 |     1
void Motor::init6motor()const
{
    wiringPiSetup();
    for (int i = 0; i < 6; i++)
    {
        pinMode(MOTOR_PINS[i], OUTPUT);
        pinMode(DIR_PINS[i], OUTPUT);
    }
}

//--------------------end class `Motor` ------------------------------------
