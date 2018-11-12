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

#endif
