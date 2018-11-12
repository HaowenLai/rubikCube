/* ********************************************************
*   This is the realization of header colorRecog.h

* @Author : Derek Lai
* @Date   : 2018/11/11
* *******************************************************/
#include "colorRecog.h"

#include <stdio.h>
#include <string.h>
#include <fcntl.h> /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>

using namespace cv;

static int device_open(const char *dev_name);
static int device_close(int fd);
static void device_busInfo(char *bus_info_actual, int fd);

//---------------------------- public function ------------------------------

//  this functoin get ordered camera and put their index into `rtCamIndex[]`
//rtCamIndex[0] is the index of down camera, rtCamIndex[1] is the index of
//up camera.
void getOrderCamera(int rtCamIndex[])
{
    const int dev_num = 2;
    const char dev_list[][15] = {"/dev/video0", "/dev/video1"};
    const char bus_info[][25] = {"usb-0000:00:14.0-4", "usb-0000:00:1d.0-1.6"};

    //get actual camera bus info
    char bus_info_actual[dev_num][25];
    for (int i = 0; i < dev_num; ++i)
    {
        int fd = device_open(dev_list[i]);
        device_busInfo(bus_info_actual[i], fd);
        device_close(fd);
    }

    //return ordered index
    for (int i = 0; i < dev_num; i++)
    {
        int index = 0;
        while (!strcmp(bus_info[i], bus_info_actual[index]))
            index++;

        rtCamIndex[i] = index;
    }
}

//  get steady state up and down image for later analyzation.
//By default it skip the first 20 frames,
void getUpDowmImg(VideoCapture &downCam, VideoCapture &upCam,
                  Mat &downImg, Mat &upImg,
                  int skipFrame)
{
    for (int i = 0; i < skipFrame; i++)
    {
        downCam >> downImg;
        upCam >> upImg;
        waitKey(20);
    }
}

//------------------------------- static function ---------------------------

static int device_open(const char *dev_name)
{
    struct stat st;
    if (-1 == stat(dev_name, &st))
    {
        fprintf(stderr, "Cannot identify '%s': %d, %s\n", dev_name, errno, strerror(errno));
        throw - 1;
    }

    if (!S_ISCHR(st.st_mode))
    {
        fprintf(stderr, "%s is no device\n", dev_name);
        throw - 1;
    }

    int fd = open(dev_name, O_RDWR /* required */ | O_NONBLOCK, 0);
    if (-1 == fd)
    {
        fprintf(stderr, "Cannot open '%s': %d, %s\n", dev_name, errno, strerror(errno));
        throw - 1;
    }

    // printf("%s %d : Open %s successfully. fd = %d\n", __func__, __LINE__, dev_name, fd);
    return fd;
}

static int device_close(int fd)
{
    if (-1 == close(fd))
    {
        printf("\tdevice close failed.\n");
        throw - 1;
    }
    // else
    //     printf("%s %d : devices close successfully\n\n", __func__, __LINE__);

    return 0;
}

static void device_busInfo(char *bus_info_actual, int fd)
{
    struct v4l2_capability cap;
    memset(&cap, 0, sizeof(cap));

    if (-1 == ioctl(fd, VIDIOC_QUERYCAP, &cap)) // query v4l2-devices's capability
    {
        if (EINVAL == errno)
        {
            fprintf(stderr, "device of fd %d is no V4L2 device\n", fd);
            throw - 1;
        }
        else
        {
            printf("\tvideo ioctl_querycap failed.\n");
            throw - 1;
        }
    }

    // printf("\n\tdriver name : %s\n\tcard name : %s\n\tbus info : %s\n\tdriver version : %u.%u.%u\n\n",
    //        cap.driver,
    //        cap.card,
    //        cap.bus_info,
    //        (cap.version >> 16) & 0XFF,
    //        (cap.version >> 8) & 0XFF,
    //        cap.version & 0XFF);
    strcpy(bus_info_actual, (char *)cap.bus_info);
}
