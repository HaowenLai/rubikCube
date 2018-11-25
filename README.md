# rubikCube
## Description
&ensp;&ensp; This project uses kociemba algorithm to solve Rubik's cube.  
&ensp;&ensp; It is used to automatically solve Rubik's cube by a device that is DIYed by my team, so it contain both software for upper monitor and lower driver(Raspberry wiringPi lib).

## Dependency & Modules
This project depends on the following libs:
- OpenCV (>= 3.3.0)
- WiringPi
- pthread

This project contains the following modules:
- Simulation
- Core solution algorithm
- Turning method(motor control)
- Color recognition
- 3D control and display

## Usage
**Before you can run this project, compile it first:**  
&ensp;&ensp; `~/rubikCube$ mkdir build && cd build`  
&ensp;&ensp; `~/rubikCube/build$ cmake ..`  
&ensp;&ensp; `~/rubikCube/build$ make`  
- simulate:  
  Use the colors of each faces as the first arugment of the program.  
  Option colors are **r b g w o y** for red, blue, green, white, orange, yellow respectively.  
  Note that the order of faces is *ULFRBD*  
  e.g. `~/rubikCube/build$ ./simulate wgbrwywwrgygwrbogwrggobwboryoyroybywoooggboryrwyrybgbb`  
  
- disorganize:  
  Generate an array of random state and implement it to the cube.  
  You can specify the number of steps by giving the second argument in the function ***genRandomState()***.  
  e.g. `~/rubikCube/build$ ./disorganize`
  
- solveCubeManually:  
  Manually input the colors of each face. The order of colors is the same as that of `simulation`.  
  e.g. `~/rubikCube/build$ ./solveCubeManually wgbrwywwrgygwrbogwrggobwboryoyroybywoooggboryrwyrybgbb`  

- colorTool:  
  Several tools about color recognition.  
  You can choose one of the tools by entering the corresponding number when the program is run.  
  
- ReadColor:  
  The program concerning color recognition.  
  You can choose whether to train a new color model or use the existing model to recognize colors.  
  By default, the model files should be in the folder the same as the path of program.  

- 3D control and display:  
  In the branch `3d_module`. This module is made in Unity3d and it is not finished yet.

## About color recognition
The color distribution is shown as pic.1  
![pic1](https://github.com/HaowenLai/rubikCube/blob/master/data/color_distribution.png)  
The recognition result is shown as pic.2  
![pic2](https://github.com/HaowenLai/rubikCube/blob/master/data/color_recognition.png)  

## Author
> Derek Lai (HaowenLai)
****
The whole device (including hardware structure and software) is really remarkable!  
The 3D control and display module can be found in branch `3d_module`. It is not finished yet.  
Have fun and enjoy it ~~
