
// ChildView.cpp : implementation of the CChildView class
//

#include "pch.h"
#include "framework.h"
#include "HonorsProject.h"
#include "ChildView.h"
#include <cmath>
#include <stdlib.h>
#include <string>
#include "CImageGen.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CChildView

CChildView::CChildView()
{
    //
    // INITIALIZATION
    //
    srand(time(NULL));

    m_camera.Set(20, 10, 50, 0, 0, 0, 0, 1, 0);

    m_WallHeight = 12;
    m_WallWidth = 24;

    m_wallBrickDepthJitterRange = 0.05;
    m_wallPolygonFactor = 3; // this is n in 2^n + 1, the values for each index (N) are 2,3,5,9,17 and so on 
    m_wallXYJitterRange = 0;//0.1;


    // 
    // TEXTURE GENERATION AND LOADING
    //

    //CImageGen im(640, 480); // not a diamond square multiple
    //CImageGen im(513, 513); // n= 9
    CImageGen im(257, 257); 
    //CImageGen im(129, 129); // alt: 257
    im.GenerateStoneTexture();
    im.Export("textures\\GeneratedStoneTexture.bmp");

    //im.ColorfulDebugImage();
    //im.Export("textures\\imageTest.bmp");

    //m_testRockTexture.LoadFile(L"textures/DefaultRockTexture.jpg");
    //m_testWorldTexture.LoadFile(L"textures/worldmap.bmp");
    m_generatedRockTexture.LoadFile(L"textures/GeneratedStoneTexture.bmp");

    
    //
    // GENERATE WALL
    //
    GenerateBricks();

    /*CStoneBrickObj* doesItPersist = new CStoneBrickObj(time(NULL), m_WallWidth, m_WallHeight, { 10., 10., 0. }, 6, 2, 1);
    m_bricks.push_back(doesItPersist);

    CStoneBrickObj* doesItPersist2 = new CStoneBrickObj(time(NULL), m_WallWidth, m_WallHeight, { 20., 10., 0. }, 6, 2, 1);
    m_bricks.push_back(doesItPersist2);*/
}

CChildView::~CChildView()
{
    for (CStoneBrickObj* brick : m_bricks) {
        delete brick;
    }
}


BEGIN_MESSAGE_MAP(CChildView, COpenGLWnd)
	ON_WM_PAINT()
    ON_WM_LBUTTONDOWN()
    ON_WM_MOUSEMOVE()
    ON_WM_RBUTTONDOWN()
END_MESSAGE_MAP()



// CChildView message handlers

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!COpenGLWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(nullptr, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), nullptr);

	return TRUE;
}


GLdouble* CrossProduct(GLdouble* vectorA, GLdouble* vectorB) {
    GLdouble xComp = (vectorA[1] * vectorB[2]) - (vectorA[2] * vectorB[1]);
    GLdouble yComp = (vectorA[2] * vectorB[0]) - (vectorA[0] * vectorB[2]);
    GLdouble zComp = (vectorA[0] * vectorB[1]) - (vectorA[1] * vectorB[0]);
    return new GLdouble[3]{ xComp, yComp, zComp };
}

void NormalizeMyVector(GLdouble* vect) {
    GLdouble magnitude = sqrt(pow(vect[0], 2) + pow(vect[1], 2) + pow(vect[2], 2));
    
    for (int i = 0; i < 3; i++) {
        vect[i] /= magnitude;
    }
}

//
//        Name : Quad()
// Description : Inline function for drawing 
//               a quadralateral.
//
inline void Quad(GLdouble* v1, GLdouble* v2, GLdouble* v3, GLdouble* v4, GLfloat numberOfTextureColumns, GLfloat numberOfTextureRows)
{
    // use v1 as origin
    GLdouble vecA[] = { v2[0] - v1[0], v2[1] - v1[1], v2[2] - v1[2] };
    GLdouble vecB[] = { v3[0] - v1[0], v3[1] - v1[1], v3[2] - v1[2] };
    GLdouble* normalVector = CrossProduct(vecA, vecB);

    NormalizeMyVector(normalVector);

    glNormal3d(normalVector[0], normalVector[1], normalVector[2]);
    
    glBegin(GL_QUADS);
    glTexCoord2f(numberOfTextureColumns, 0);
    glVertex3dv(v1);
    glTexCoord2f(numberOfTextureColumns, numberOfTextureRows);
    glVertex3dv(v2);
    glTexCoord2f(0, numberOfTextureRows);
    glVertex3dv(v3);
    glTexCoord2f(0, 0);
    glVertex3dv(v4);
    glEnd();

    delete[] normalVector;
}

inline void Triangle(GLdouble* v1, GLdouble* v2, GLdouble* v3)
{
    // use v1 as origin
    GLdouble vecA[] = { v2[0] - v1[0], v2[1] - v1[1], v2[2] - v1[2] };
    GLdouble vecB[] = { v3[0] - v1[0], v3[1] - v1[1], v3[2] - v1[2] };
    GLdouble* normalVector = CrossProduct(vecA, vecB);

    NormalizeMyVector(normalVector);

    glNormal3d(normalVector[0], normalVector[1], normalVector[2]);
    glBegin(GL_POLYGON);
    glVertex3dv(v1);
    glVertex3dv(v2);
    glVertex3dv(v3);
    glEnd();

    delete[] normalVector;
}

inline void BrickTriangle(GLdouble* v1, GLdouble* v2, GLdouble* v3, GLdouble* tv1, GLdouble* tv2, GLdouble* tv3)
{
    // use v1 as origin
    GLdouble vecA[] = { v2[0] - v1[0], v2[1] - v1[1], v2[2] - v1[2] };
    GLdouble vecB[] = { v3[0] - v1[0], v3[1] - v1[1], v3[2] - v1[2] };
    GLdouble* normalVector = CrossProduct(vecA, vecB);

    NormalizeMyVector(normalVector);

    glNormal3d(normalVector[0], normalVector[1], normalVector[2]);
    glBegin(GL_POLYGON);
    glTexCoord2f(tv1[0], tv1[1]);
    glVertex3dv(v1);
    glTexCoord2f(tv2[0], tv2[1]);
    glVertex3dv(v2);
    glTexCoord2f(tv3[0], tv3[1]);
    glVertex3dv(v3);
    glEnd();

    delete[] normalVector;
}

double GetRand(double min, double max) {
    return min + ((((double)rand()) / RAND_MAX) * (max - min));
}

double RandomBounded(double initial, double max) {
    double result = initial + GetRand(-0.5, 0.5);

    if (result < 0) result = 0; // assuming 0 is the min
    if (result > max) result = max;

    return result;
}

void CChildView::GenerateBricks() {

    double brickWidth = 5;
    double brickHeight = 2;
    double brickDepth = 1;

    double spacing = 0.5;
    double bricksPerRow = m_WallWidth / (brickWidth + spacing);
    double numberOfRows = m_WallHeight / (brickHeight + spacing);

    for (int j = 0; j < numberOfRows; j++)
    {
        for (int i = 0; i < bricksPerRow; i++) {
            double horizontalStagger = j % 2 == 0 ? (brickWidth / 2) : (brickWidth + spacing / 2);
            horizontalStagger -= (brickWidth / 2);

            std::vector<GLdouble> brickPosition = { 
                horizontalStagger + (i * (brickWidth + spacing)),
                (brickHeight / 2) + (j * (brickHeight + spacing)),
                0. 
            };

            // double wallWidth, double wallHeight, std::vector<GLdouble> brickPosition, double brickWidth, double brickHeight, double brickDepth
            CStoneBrickObj* newBrick = new CStoneBrickObj(GetRand(1, 10000), m_WallWidth, m_WallHeight, brickPosition, brickWidth, brickHeight, brickDepth);
            m_bricks.push_back(newBrick);
        }
    }   
}

double FindDistance(GLdouble* point1, GLdouble* point2) {
    return sqrt(pow(point2[0] - point1[0], 2) + pow(point2[1] - point1[1], 2) + pow(point2[2] - point1[2], 2));
}

GLdouble* VectorToGLdoubleArray(std::vector<GLdouble> vec) {
    return new GLdouble[3]{ vec[0], vec[1], vec[2] };
}

inline void CChildView::XYCoordsToQuad(VertexArray2D* bigArray, XYCoord topLeft, XYCoord bottomLeft, XYCoord bottomRight, XYCoord topRight, int xMax)
{
    GLdouble* topLeftArray = VectorToGLdoubleArray((*bigArray)[topLeft.x][topLeft.y]);
    GLdouble* bottomLeftArray = VectorToGLdoubleArray((*bigArray)[bottomLeft.x][bottomLeft.y]);
    GLdouble* bottomRightArray = VectorToGLdoubleArray((*bigArray)[bottomRight.x][bottomRight.y]);
    GLdouble* topRightArray = VectorToGLdoubleArray((*bigArray)[topRight.x][topRight.y]);

    for (GLdouble* vertex : { topLeftArray, bottomLeftArray ,bottomRightArray ,topRightArray }) {
        if (vertex[2] < 0) { // if z is less than 0
            vertex[2] = 0;
        }
    }

    xMax /= 2;
    GLdouble topLeftTextureCoords[] = { (double)topLeft.x / (double)xMax, (double)topLeft.y / (double)xMax };
    GLdouble bottomLeftTextureCoords[] = { (double)bottomLeft.x / (double)xMax, (double)bottomLeft.y / (double)xMax };
    GLdouble bottomRightTextureCoords[] = { (double)bottomRight.x / (double)xMax, (double)bottomRight.y / (double)xMax };
    GLdouble topRightTextureCoords[] = { (double)topRight.x / (double)xMax, (double)topRight.y / (double)xMax };

    //BrickTriangle()
    BrickTriangle(topLeftArray, bottomLeftArray, bottomRightArray, topLeftTextureCoords, bottomLeftTextureCoords, bottomRightTextureCoords);
    BrickTriangle(topLeftArray, bottomRightArray, topRightArray, topLeftTextureCoords, bottomRightTextureCoords, topRightTextureCoords);

    delete[] topLeftArray;
    delete[] bottomLeftArray;
    delete[] bottomRightArray;
    delete[] topRightArray;
}


inline void CChildView::RenderBrickSurface() {

    // for each brick
    for(CStoneBrickObj* brick : m_bricks)
    {
        for (int x = 1; x < brick->m_squareSideLength; x++)
        {
            for (int y = 1; y < brick->m_squareSideLength; y++)
            {
                // XYCoord topLeft, XYCoord bottomLeft, XYCoord bottomRight, XYCoord topRight
                XYCoordsToQuad(&brick->m_vertices, XYCoord(x - 1, y), XYCoord(x - 1, y - 1), XYCoord(x, y - 1), XYCoord(x, y), brick->m_squareSideLength-1);

                if (y > brick->m_squareSideLength / 2) continue;
                XYCoordsToQuad(&brick->m_verticesTop, XYCoord(x - 1, y), XYCoord(x - 1, y - 1), XYCoord(x, y - 1), XYCoord(x, y), brick->m_squareSideLength-1);
                XYCoordsToQuad(&brick->m_verticesLeft, XYCoord(x - 1, y), XYCoord(x - 1, y - 1), XYCoord(x, y - 1), XYCoord(x, y), brick->m_squareSideLength-1);
                XYCoordsToQuad(&brick->m_verticesRight, XYCoord(x - 1, y), XYCoord(x - 1, y - 1), XYCoord(x, y - 1), XYCoord(x, y), brick->m_squareSideLength-1);
                XYCoordsToQuad(&brick->m_verticesBottom, XYCoord(x - 1, y), XYCoord(x - 1, y - 1), XYCoord(x, y - 1), XYCoord(x, y), brick->m_squareSideLength-1);
            }
        }
    }

    // Flat panels
    /*XYCoordsToQuad(&m_bricks[0]->m_verticesTop, 
        XYCoord(0, m_bricks[0]->m_squareSideLength - 1),
        XYCoord(0, 0), 
        XYCoord(m_bricks[0]->m_squareSideLength - 1, 0), 
        XYCoord(m_bricks[0]->m_squareSideLength - 1, m_bricks[0]->m_squareSideLength - 1));*/
}

void CChildView::OnGLDraw(CDC* pDC)
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //
    // Set up the camera
    //
    int width, height;
    GetSize(width, height);
    m_camera.Apply(width, height);

    gluLookAt(5, 5, 5,    // eye x,y,z
               0., 0., 0.,       // center x,y,z
               0., 1., 0.);      // Up direction

    //
    // Some standard parameters
    //

    // Enable depth test
    glEnable(GL_DEPTH_TEST);

    // Cull backfacing polygons
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);

    // Enable lighting
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    GLfloat lightpos[] = { 50, 50, 50, 1. };
    glLightfv(GL_LIGHT0, GL_POSITION, lightpos);

    // Draw a coordinate axis
    //glColor3d(0., 1., 1.);
    GLfloat lightBlue[] = { 0.f, 1.f, 1.f, 1.f };
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, lightBlue);

    glBegin(GL_LINES);
    glVertex3d(0., 0., 0.);
    glVertex3d(12., 0., 0.);
    glVertex3d(0., 0., 0.);
    glVertex3d(0., 12., 0.);
    glVertex3d(0., 0., 0.);
    glVertex3d(0., 0., 12.);
    glEnd();


    // 
    // INSERT DRAWING CODE HERE
    //

    // Eventually this will need to be bigger in order to incorporate the greater number of vertices

    GLdouble wallV1[] = { m_WallWidth - 0.3, 0., 0. };
    GLdouble wallV2[] = { m_WallWidth - 0.3, m_WallHeight - 0.3, 0. };
    GLdouble wallV3[] = { 0., m_WallHeight-0.3, 0. };
    GLdouble wallV4[] = { 0., 0., 0. };

    GLfloat mortarGrey[] = { 0.15f, 0.15f, 0.15f, 1.f };
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mortarGrey);
    
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D, m_generatedRockTexture.TexName());
    glEnable(GL_TEXTURE_2D);
    Quad(wallV1, wallV2, wallV3, wallV4, m_WallWidth, m_WallHeight);
    Quad(wallV4, wallV3, wallV2, wallV1, m_WallWidth, m_WallHeight);
    glDisable(GL_TEXTURE_2D);
    
    GLfloat brickColor[] = { 0.6f, .6f, .6f, 1.f };
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, brickColor);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_generatedRockTexture.TexName());

    RenderBrickSurface();

    glDisable(GL_TEXTURE_2D);


}


void CChildView::OnLButtonDown(UINT nFlags, CPoint point)
{
    m_camera.MouseDown(point.x, point.y);

    COpenGLWnd::OnLButtonDown(nFlags, point);
}


void CChildView::OnMouseMove(UINT nFlags, CPoint point)
{
    if (m_camera.MouseMove(point.x, point.y, nFlags))
        Invalidate();

    COpenGLWnd::OnMouseMove(nFlags, point);
}


void CChildView::OnRButtonDown(UINT nFlags, CPoint point)
{
    m_camera.MouseDown(point.x, point.y, 2);

    COpenGLWnd::OnRButtonDown(nFlags, point);
}
