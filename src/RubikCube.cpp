/* ********************************************************
*   This is the realization of class `RubikCube` and `Face`

* @Author : Derek Lai
* @Date   : 2018/9/21
* *******************************************************/

#include "RubikCube.h"

using namespace cv;
using namespace std;

//----------------------------- class `Face` begins --------------------------------
//public:

const int Face::pieceColorSize = 16;             //16*16 filled with color
const int Face::pieceGapSize = 2;                //2     the gray gap
const Mat Face::redPiece   (pieceColorSize, pieceColorSize, CV_8UC3, Scalar(0, 0, 255));
const Mat Face::greenPiece (pieceColorSize, pieceColorSize, CV_8UC3, Scalar(0, 255, 0));
const Mat Face::bluePiece  (pieceColorSize, pieceColorSize, CV_8UC3, Scalar(255, 0, 0));
const Mat Face::yellowPiece(pieceColorSize, pieceColorSize, CV_8UC3, Scalar(0, 255, 255));
const Mat Face::whitePiece (pieceColorSize, pieceColorSize, CV_8UC3, Scalar(255, 255, 255));
const Mat Face::orangePiece(pieceColorSize, pieceColorSize, CV_8UC3, Scalar(0, 120, 255));


//  Display the color of each piece of the face with its 
//first left-top point at (x,y) on img
void Face::display(cv::Mat& img, int x, int y)const
{
    const int bgL = pieceColorSize * 3 + pieceGapSize * 4;     //backgrand length
    Mat backgrand(bgL, bgL, CV_8UC3, Scalar(161, 155, 160));

    for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++)
    {
        Rect area((j + 1)*pieceGapSize + j*pieceColorSize,
                  (i + 1)*pieceGapSize + i*pieceColorSize,
                  pieceColorSize, pieceColorSize);
        
        switch (data[i * 3 + j])
        {
        case RED:
            redPiece.copyTo(backgrand(area));
            break;
        case GREEN:
            greenPiece.copyTo(backgrand(area));
            break;
        case BLUE:
            bluePiece.copyTo(backgrand(area));
            break;
        case YELLOW:
            yellowPiece.copyTo(backgrand(area));
            break;
        case WHITE:
            whitePiece.copyTo(backgrand(area));
            break;
        case ORANGE:
            orangePiece.copyTo(backgrand(area));
            break;
        default:
            break;
        }
    }

    backgrand.copyTo(img(Rect(x, y, bgL, bgL)));
}


//  This function set colors for the face. The color listed
//in color[] is from left to right, up to down.
//NOTE:color[] must have 9 elements!
void Face::setColor(const Face::COLOR color[])
{
    for (int i = 0; i < 9; i++)
    {
        data[i] = color[i];
    }
    centerColor = data[4];
}


//turn the face clockwise 90 degrees (-90 degrees)
void Face::turn_90()
{
    //transform of the corners
    COLOR temp = data[0];
    data[0] = data[6];
    data[6] = data[8];
    data[8] = data[2];
    data[2] = temp;

    //transform of the edges
    temp = data[1];
    data[1] = data[3];
    data[3] = data[7];
    data[7] = data[5];
    data[5] = temp;
}

//turn the face counterclockwise 90 degrees (90 degrees)
void Face::turn90()
{
    //transform of the corners
    COLOR temp = data[0];
    data[0] = data[2];
    data[2] = data[8];
    data[8] = data[6];
    data[6] = temp;

    //transform of the edges
    temp = data[1];
    data[1] = data[5];
    data[5] = data[7];
    data[7] = data[3];
    data[3] = temp;
}

//turn the face 180 degrees
void Face::turn180()
{
    //transform of the corners
    COLOR temp = data[0];
    data[0] = data[8];
    data[8] = temp;
    temp = data[2];
    data[2] = data[6];
    data[6] = temp;

    //transform of the edges
    temp = data[1];
    data[1] = data[7];
    data[7] = temp;
    temp = data[3];
    data[3] = data[5];
    data[5] = temp;
}

//----------------------------- class `Face` ends --------------------------------





//----------------------------- class `RubikCube` begins --------------------------------
//public

//Adjancent face index for function `turnAdj4faces`
// naming rule: when you face with the named face, for example, the
//front face, its adjancent face will be rank clockwisely  or conterclockwisely
//with 12 o'clock as its first face, which is the up face in this instance.
//--clockwisely
const int RubikCube::frontIndex_90[]{6,7,8,0,3,6,2,1,0,8,5,2};
const int RubikCube::backIndex_90[] {2,1,0,0,3,6,6,7,8,8,5,2};
const int RubikCube::leftIndex_90[] {0,3,6,0,3,6,0,3,6,8,5,2};
const int RubikCube::rightIndex_90[]{8,5,2,0,3,6,8,5,2,8,5,2};
const int RubikCube::upIndex_90[]   {2,1,0,2,1,0,2,1,0,2,1,0};
const int RubikCube::downIndex_90[] {6,7,8,6,7,8,6,7,8,6,7,8};
//--conterclockwisely
const int RubikCube::frontIndex90[]{8,7,6,2,5,8,0,1,2,6,3,0};
const int RubikCube::backIndex90[] {0,1,2,2,5,8,8,7,6,6,3,0};
const int RubikCube::leftIndex90[] {6,3,0,2,5,8,6,3,0,6,3,0};
const int RubikCube::rightIndex90[]{2,5,8,2,5,8,2,5,8,6,3,0};
const int RubikCube::upIndex90[]   {0,1,2,0,1,2,0,1,2,0,1,2};
const int RubikCube::downIndex90[] {8,7,6,8,7,6,8,7,6,8,7,6};
//--180 degrees
//first 6 index belong to fa, and the last 6 belong to fb
//!fa1,fb1,fa2,fb2 are ranked conterclockwisely
const int RubikCube::frontIndex180[]{8,7,6,0,1,2,2,5,8,6,3,0};
const int RubikCube::backIndex180[] {0,1,2,8,7,6,2,5,8,6,3,0};
const int RubikCube::leftIndex180[] {6,3,0,6,3,0,2,5,8,6,3,0};
const int RubikCube::rightIndex180[]{2,5,8,2,5,8,2,5,8,6,3,0};
const int RubikCube::upIndex180[]   {0,1,2,0,1,2,0,1,2,0,1,2};
const int RubikCube::downIndex180[] {8,7,6,8,7,6,8,7,6,8,7,6};


RubikCube::RubikCube()
{
    turnMethod[0]  = &RubikCube::turnF_90;
    turnMethod[1] = &RubikCube::turnF90;
    turnMethod[2] = &RubikCube::turnF180;
    turnMethod[3] = &RubikCube::turnB_90;
    turnMethod[4] = &RubikCube::turnB90;
    turnMethod[5] = &RubikCube::turnB180;
    turnMethod[6] = &RubikCube::turnL_90;
    turnMethod[7] = &RubikCube::turnL90;
    turnMethod[8] = &RubikCube::turnL180;
    turnMethod[9] = &RubikCube::turnR_90;
    turnMethod[10] = &RubikCube::turnR90;
    turnMethod[11] = &RubikCube::turnR180;
    turnMethod[12] = &RubikCube::turnU_90;
    turnMethod[13] = &RubikCube::turnU90;
    turnMethod[14] = &RubikCube::turnU180;
    turnMethod[15] = &RubikCube::turnD_90;
    turnMethod[16] = &RubikCube::turnD90;
    turnMethod[17] = &RubikCube::turnD180;
}


//display the colors of six faces on img.
//    (x,y)     ---->x
//  |    .-----------
//  |    |   U      |
//  v    |L  F  R  B|
//  y    |   D      |
//       ------------
void RubikCube::display(cv::Mat& img, int x, int y)const
{
    const int faceSz = Face::pieceColorSize * 3 + Face::pieceGapSize * 4;
    const int divideSz = 2;

    //draw each face
    upFace.display(img, x + faceSz + divideSz, y);
    leftFace.display(img, x, y + faceSz + divideSz);
    frontFace.display(img, x + faceSz + divideSz, y + faceSz + divideSz);
    rightFace.display(img, x + 2 * (faceSz + divideSz), y + faceSz + divideSz);
    backFace.display(img, x + 3 * (faceSz + divideSz), y + faceSz + divideSz);
    downFace.display(img, x + faceSz + divideSz, y + 2 * (faceSz + divideSz));

    //draw divide black line
    Scalar divideLineColor(0, 0, 0);  //black divide line
    line(img, Point(x + faceSz, y + faceSz), Point(x + 2 * faceSz + divideSz, y + faceSz),
        divideLineColor, 2);
    line(img, Point(x + faceSz, y + faceSz), Point(x + faceSz, y + 2 * faceSz + divideSz),
        divideLineColor, 2);
    line(img, Point(x + faceSz, y + 2 * faceSz + divideSz), Point(x + 2 * faceSz + divideSz, y + 2 * faceSz + divideSz),
        divideLineColor, 2);
    line(img, Point(x + 2 * faceSz + divideSz, y + faceSz), Point(x + 2 * faceSz + divideSz, y + 2 * faceSz + divideSz),
        divideLineColor, 2);
    line(img, Point(x + 3 * faceSz + 2 * divideSz, y + faceSz), Point(x + 3 * faceSz + 2 * divideSz, y + 2 * faceSz + divideSz),
        divideLineColor, 2);
}


//the rank is ULFRBD      U
//                    L   F   R   B
//                        D
//NOTE:color[] must have 54 elements!
void RubikCube::setAllFaceColor(Face::COLOR color[])
{
    Face::COLOR* p = color;
    upFace.setColor   (p);
    leftFace.setColor (p += 9);
    frontFace.setColor(p += 9);
    rightFace.setColor(p += 9);
    backFace.setColor (p += 9);
    downFace.setColor (p += 9);
}


///@ the turning face method group
//| | | | | | | | | | | | | | | | | | | | 
//v v v v v v v v v v v v v v v v v v v v
void RubikCube::turnF_90()
{
    frontFace.turn_90();
    turnAdj4faces(upFace, rightFace, downFace, leftFace, frontIndex_90);
}

void RubikCube::turnF90()
{
    frontFace.turn90();
    turnAdj4faces(upFace, leftFace, downFace, rightFace, frontIndex90);
}

void RubikCube::turnF180()
{
    frontFace.turn180();
    turnOpp4faces(upFace, downFace, leftFace, rightFace, frontIndex180);
}

void RubikCube::turnB_90()
{
    backFace.turn_90();
    turnAdj4faces(upFace, leftFace, downFace, rightFace, backIndex_90);
}

void RubikCube::turnB90()
{
    backFace.turn90();
    turnAdj4faces(upFace, rightFace, downFace, leftFace, backIndex90);
}

void RubikCube::turnB180()
{
    backFace.turn180();
    turnOpp4faces(upFace, downFace, rightFace, leftFace, backIndex180);
}

void RubikCube::turnL_90()
{
    leftFace.turn_90();
    turnAdj4faces(upFace, frontFace, downFace, backFace, leftIndex_90);
}

void RubikCube::turnL90()
{
    leftFace.turn90();
    turnAdj4faces(upFace, backFace, downFace, frontFace, leftIndex90);
}

void RubikCube::turnL180()
{
    leftFace.turn180();
    turnOpp4faces(upFace, downFace, backFace, frontFace, leftIndex180);
}

void RubikCube::turnR_90()
{
    rightFace.turn_90();
    turnAdj4faces(upFace, backFace, downFace, frontFace, rightIndex_90);
}

void RubikCube::turnR90()
{
    rightFace.turn90();
    turnAdj4faces(upFace, frontFace, downFace, backFace, rightIndex90);
}

void RubikCube::turnR180()
{
    rightFace.turn180();
    turnOpp4faces(upFace, downFace, frontFace, backFace, rightIndex180);
}

void RubikCube::turnU_90()
{
    upFace.turn_90();
    turnAdj4faces(backFace, rightFace, frontFace, leftFace, upIndex_90);
}

void RubikCube::turnU90()
{
    upFace.turn90();
    turnAdj4faces(backFace, leftFace, frontFace, rightFace, upIndex90);
}

void RubikCube::turnU180()
{
    upFace.turn180();
    turnOpp4faces(backFace, frontFace, leftFace, rightFace, upIndex180);
}

void RubikCube::turnD_90()
{
    downFace.turn_90();
    turnAdj4faces(frontFace, rightFace, backFace, leftFace, downIndex_90);
}

void RubikCube::turnD90()
{
    downFace.turn90();
    turnAdj4faces(frontFace, leftFace, backFace, rightFace, downIndex90);
}

void RubikCube::turnD180()
{
    downFace.turn180();
    turnOpp4faces(frontFace, backFace, leftFace, rightFace, downIndex180);
}
//^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ 
//| | | | | | | | | | | | | | | | | | | |



//private:

//  turn the adjacent four faces of the main face(the face you
//are manipulating). It move index[0,1,2] of f1 to index[3,4,5] of f2,
//and index[3,4,5] of f2 to index[6,7,8] to f3....and etc.
// f1 -> f2 -> f3 -> f4 ->f1
//Note: std::vector<int>& index must have 12 elements.
void RubikCube::turnAdj4faces(Face& f1, Face& f2, Face& f3, Face& f4, const int* index)
{
    Face::COLOR temp[3];
    //f4 -> f1
    for (int i = 0; i < 3; i++)
    {
        temp[i] = f1.data[index[i]];
        f1.data[index[i]] = f4.data[index[i + 9]];
    }

    //f3 -> f4
    for (int i = 9; i < 12; i++)
        f4.data[index[i]] = f3.data[index[i - 3]];
    
    //f2 -> f3
    for (int i = 6; i < 9; i++)
        f3.data[index[i]] = f2.data[index[i - 3]];

    //f1 -> f2
    for (int i = 3; i < 6; i++)
        f2.data[index[i]] = temp[i - 3];
}


//  turn the oppsite four faces of the main face(the face you
//are manipulating). It move index[0,1,2] of fa1 to index[3,4,5] of fa2,
//and index[6,7,8] of fb1 to index[9,10,11] to fb2.
// fa1 <-> fa2, fb1 <-> fb2,   fa1,fb1,fa2,fb2 are ranked conterclockwisely.
//Note: index[] must have 12 elements.
void RubikCube::turnOpp4faces(Face &fa1, Face &fa2, Face &fb1, Face &fb2, const int *index)
{
    Face::COLOR temp;
    //fa1 <-> fa2
    for (int i = 0; i < 3; i++)
    {
        temp = fa1.data[index[i]];
        fa1.data[index[i]] = fa2.data[index[i + 3]];
        fa2.data[index[i + 3]] = temp;
    }

    //fb1 <-> fb2
    for (int i = 6; i < 9; i++)
    {
        temp = fb1.data[index[i]];
        fb1.data[index[i]] = fb2.data[index[i + 3]];
        fb2.data[index[i + 3]] = temp;
    }
}

//----------------------------- class `RubikCube` ends --------------------------------

