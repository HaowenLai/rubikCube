#include <iostream>
#include "wiringPi.h"

int main(int argc, char **argv)
{
    const unsigned int envPin = 25;
    wiringPiSetup();
    pinMode(envPin, OUTPUT);

    std::cout << "enter 1 to disable six motors, 0 to enable six motors\n";

    int v;
    while (1)
    {
        std::cout << "please enter your choice:\n";
        std::cin >> v;
        if(v)
            digitalWrite(envPin, HIGH);
        else
            digitalWrite(envPin, LOW);

        std::cout << "OK!..\n";
    }

    return 0;
}