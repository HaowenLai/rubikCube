/* *******************************************************
 *   The header contains function to control the motors that
 * turn each face of the rubik's cube.
 *   The class `motor` contains methods to manipulate 6 motors. 
 * 
 * @Author : Derek Lai
 * @Date   : 2018/11/7
 * ******************************************************/

#ifndef __CONTROL_H__
#define __CONTROL_H__

#include <vector>

class Motor
{
 public:

    //construct function. Init 6 motors(12 pins)
    Motor(unsigned int peri, unsigned int delayT);

    //drive 6 motors according to an array `steps`
    //e.g. `steps` = 0 4 8 11 1 3 .....
    //acutual movement is U_90, L90, F180.....
    //see more relation at file 'RubikCube.h'
    void drive6motor(const std::vector<int> &steps) const;

 private:
    //change motor direction
    void changeDirection(int dirPin, int direction) const;

    //drive two opposite motors. For optimization usage.
    void driveOppMotors(int motorPin1, int motorPin2,
                        int degree1,   int degree2)const;
    
    //drive the motor specify by `motorNum` to turn `degree`
    void driveSingleMotor(int motorPin, int degree)const;


    //get 12 pins ready for output.
    //Wiring Pi: 0  1  | 2  3  | 4  5  | 6  7 | 21 22 | 23 24
    //board pin: 11 12 | 13 15 | 16 18 | 22 7 | 29 31 | 33 35
    //motor    :   U   |   L   |   F   |   R  |   B   |   D
    //dir+     : 1     | 1     | 1     | 1    | 1     | 1
    //pul+     :     1 |     1 |     1 |    1 |     1 |     1
    void init6motor()const;

 public:
    // define names for all motor pins
    //        ------->                   U  L  F  R  B   D
    static const int MOTOR_PINS[]; // = {1, 3, 5, 7, 22, 24};
    static const int DIR_PINS[];   // = {0, 2, 4, 6, 21, 23};

    //define names for direction
    static const int MT_CCW; //counter-clockwise
    static const int MT_CW;  //clockwise

 private:
    //you can change the speed by these two variables.
    //They are initialized by construct function by default.
    const unsigned int period;
    const unsigned int delayTime;
};

#endif
