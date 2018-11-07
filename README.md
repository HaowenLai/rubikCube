# rubikCube
## description
This project uses kociemba algorithm to solve Rubik's cube.  
It is used to automatically solve Rubik's cube by a device that is DIYed by my team, so it contain both software for upper monitor and lower driver(Raspberry wiringPi lib).

## dependency & modules
This project depends on the following libs:
- OpenCV (>= 3.3.0)
- WiringPi
- pthread

This project contains the following modules:
- Simulation
- Random state generation
- 3D turning effect
- Raspberry driver

## usage
**Before you can run this project, compile it first:**  
&ensp;&ensp; `~/rubikCube$ mkdir build && cd build`  
&ensp;&ensp; `~/rubikCube/build$ cmake ..`  
&ensp;&ensp; `~/rubikCube/build$ make`  
- simulate:
  use the colors of each faces as the first arugment of the program.  
  Option colors are **r b g w o y** for red, blue, green, white, orange, yellow respectively.  
  Note that the order of faces is *ULFRBD*  
  e.g. `~/rubikCube/build$ ./simulate wgbrwywwrgygwrbogwrggobwboryoyroybywoooggboryrwyrybgbb`

## author
> Derek Lai (HaowenLai)
****
The whole device (including hardware structure and software) is really remarkable!  
Have fun and enjoy it ~~
