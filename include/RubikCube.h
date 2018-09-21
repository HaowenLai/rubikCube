/* *******************************************************
 *   The class `RubikCube` is designed for the realworld
 * Rubik's cube. It has some basic properties such as
 * Faces, and some basic methods such as turn_xxx() and
 * display().
 *   The class `Face` is designed for the faces of rubik's
 * cube.
 * @Author : Derek Lai
 * @Date   : 2018/9/21
 * ******************************************************/
#ifndef __RUBIKCUBE_H__
#define __RUBIKCUBE_H__

#include <opencv2/opencv.hpp>
#include <vector>

class Face
{
 public:
    enum COLOR
    {
        RED,
        GREEN,
        BLUE,
        YELLOW,
        WHITE,
        ORANGE
    };
    
    //  Display the color of each piece of the face with its 
    //first left-top point at (x,y) on img
    void display(cv::Mat& img, int x, int y)const;

    // This function set colors for the face. The color listed
    //in color[] is from left to right, up to down.
    //NOTE:color[] must have 9 elements!
    void setColor(const Face::COLOR color[]);
    
    //turn the face clockwise 90 degrees (-90 degrees)
    void turn_90();

    //turn the face counterclockwise 90 degrees (90 degrees)
    void turn90();

    //turn the face 180 degrees
    void turn180();

 public:
    static const int pieceGapSize;          //the gray gap
    static const int pieceColorSize;        //piece that filled with color

    COLOR centerColor;
    COLOR data[9];   //the left top piece is No. 1, from left to right, up to down

 private:
    static const cv::Mat redPiece;          //color pieces
    static const cv::Mat greenPiece;
    static const cv::Mat bluePiece;
    static const cv::Mat yellowPiece;
    static const cv::Mat whitePiece;
    static const cv::Mat orangePiece;
    
};




class RubikCube
{
 public:
    //construct function
    RubikCube();
    
    //display the colors of six faces on img.
    //    (x,y)
    //       .-----------
    //       |   U      |
    //       |L  F  R  B|
    //       |   D      |
    //       ------------
    void display(cv::Mat& img, int x, int y)const;
    
    
    //the rank is ULFRBD      U
    //                    L   F   R   B
    //                        D
    //NOTE:color[] must have 54 elements!
    void setAllFaceColor(Face::COLOR color[]);

    //turn front face...
    void turnF_90(); //90 degrees clockwisely
    void turnF90();  //90 degrees conterclockwisely
    void turnF180(); //180 degrees

    //turn back face...
    void turnB_90(); //90 degrees clockwisely
    void turnB90();  //90 degrees conterclockwisely
    void turnB180(); //180 degrees

    //turn left face...
    void turnL_90(); //90 degrees clockwisely
    void turnL90();  //90 degrees conterclockwisely
    void turnL180(); //180 degrees

    //turn right face...
    void turnR_90(); //90 degrees clockwisely
    void turnR90();  //90 degrees conterclockwisely
    void turnR180(); //180 degrees

    //turn up face...
    void turnU_90(); //90 degrees clockwisely
    void turnU90();  //90 degrees conterclockwisely
    void turnU180(); //180 degrees

    //turn down face...
    void turnD_90(); //90 degrees clockwisely
    void turnD90();  //90 degrees conterclockwisely
    void turnD180(); //180 degrees

 private:
    //  turn the adjacent four faces of the main face(the face you
    //are manipulating). It move index[0,1,2] of f1 to index[3,4,5] of f2,
    //and index[3,4,5] of f2 to index[6,7,8] to f3....and etc.
    // f1 -> f2 -> f3 -> f4 ->f1
    //Note: index[] must have 12 elements.
    void turnAdj4faces(Face& f1,Face& f2,Face& f3,Face& f4,const int* index);

    //  turn the oppsite four faces of the main face(the face you
    //are manipulating). It move index[0,1,2] of fa1 to index[3,4,5] of fa2,
    //and index[6,7,8] of fb1 to index[9,10,11] to fb2.
    // fa1 <-> fa2, fb1 <-> fb2,   fa1,fb1,fa2,fb2 are ranked conterclockwisely.
    //Note: index[] must have 12 elements.
    void turnOpp4faces(Face &fa1, Face &fa2, Face &fb1, Face &fb2, const int *index);

 public:
    //function pointer, pointed to each turning method following the above rank.
    void (RubikCube::*turnMethod[18])();

 private:
    Face frontFace;
    Face backFace;
    Face leftFace;
    Face rightFace;
    Face upFace;
    Face downFace;

    //Adjancent face index for function `turnAdj4faces`
    // naming rule: when you face with the named face, for example, the
    //front face, its adjancent face will be rank clockwisely  or conterclockwisely
    //with 12 o'clock as its first face, which is the up face in this instance.
    //--clockwisely
    const static int frontIndex_90[];
    const static int backIndex_90[];
    const static int leftIndex_90[];
    const static int rightIndex_90[];
    const static int upIndex_90[];
    const static int downIndex_90[];
    //--conterclockwisely
    const static int frontIndex90[];
    const static int backIndex90[];
    const static int leftIndex90[];
    const static int rightIndex90[];
    const static int upIndex90[];
    const static int downIndex90[];
    //--180 degrees
    //first 6 index belong to fa, and the last 6 belong to fb
    //!fa1,fb1,fa2,fb2 are ranked conterclockwisely
    const static int frontIndex180[];
    const static int backIndex180[];
    const static int leftIndex180[];
    const static int rightIndex180[];
    const static int upIndex180[];
    const static int downIndex180[];
};


#endif

