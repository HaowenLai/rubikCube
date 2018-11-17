/* *******************************************************
 *   The header contains functions about color recognition
 * of the rubik's cube.
 *   Besides, functiuons of video devices selection are
 * available as well. 
 * 
 * @Author : Derek Lai
 * @Date   : 2018/11/11
 * ******************************************************/

#ifndef __COLORRECOG_H__
#define __COLORRECOG_H__

#include <opencv2/opencv.hpp>

//  this functoin get ordered camera and put their index into `rtCamIndex[]`
//rtCamIndex[0] is the index of down camera, rtCamIndex[1] is the index of 
//up camera.
void getOrderCamera(int rtCamIndex[]);

//  get steady state up and down image for later analyzation.
//By default it skip the first 20 frames,
void getUpDowmImg(cv::VideoCapture &downCam, cv::VideoCapture &upCam,
                  cv::Mat &downImg, cv::Mat &upImg,
                  int skipFrame = 20);

// Infer the unknown color of an angle-block.
//For every angle-block, the color unknown can be infered if the rest two colors are clear.
//Colors are represented by letters 'wrbogy', and the unknow is represented by 'x'.
//NOTE: up-w, left-r, front-b
void inferUnknownClr(char colorLetters[]);

// train svm models for up and down cameras respectively to recognize colors.
void trainColorModel(cv::VideoCapture &downCam, cv::VideoCapture &upCam,
                     std::string modelPath = std::string("./"));

// Read all six faces colors.
//This is an overload function using SVM to read colors
//The output rank is the same as that defined in class `RubikCube`.
//e.g. "rgbowyyoow....."
void read6faceColor(cv::VideoCapture &downCam, cv::VideoCapture &upCam,
                    char colorLetter[],
                    std::string modelPath = std::string("./"));

// Read all six faces colors.
//Input images from up and down cameras, and output the color letter array.
//The output rank is the same as that defined in class `RubikCube`.
//e.g. "rgbowyyoow....."
void read6faceColor(cv::Mat &upImg, cv::Mat &downImg, char colorLetter[]);

#endif
