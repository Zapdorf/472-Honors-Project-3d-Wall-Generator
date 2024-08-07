#include "pch.h"
#include "CStoneBrickObj.h"


#define PI 3.14159265

CStoneBrickObj::CStoneBrickObj(double seed, double wallWidth, double wallHeight, std::vector<GLdouble> brickPosition, double brickWidth, double brickHeight, double brickDepth)
{
	// calculate vertices based on parameters

    // initialize randomness
    srand(seed);
    
    m_WallHeight = wallHeight; //24;
    m_WallWidth = wallWidth; //48;

    m_wallBrickDepthJitterRange = 0.25; //0.2 looks great, 0.08 is safe
    m_wallPolygonFactor = 3; // this is n in 2^n + 1, the values for each index (N) are 2,3,5,9,17 and so on 
    m_wallXYJitterRange = 0;//0.1;

    m_brickShapeJitterRange = 0.35;

    // Point per unit, drop to int, use smallest to create a square
    m_squareSideLength = pow(2, m_wallPolygonFactor) + 1; // must be 2^n + 1

    m_postFrontDepthBoost = 0;

    // initialize 3D array 
    InitializeVertexList(&m_vertices);
    InitializeVertexList(&m_verticesTop);
    InitializeVertexList(&m_verticesLeft);
    InitializeVertexList(&m_verticesRight);
    InitializeVertexList(&m_verticesBottom);
    
	// save the vertices so that these aren't recalculated on draw
    GenerateVertices(brickPosition, brickWidth, brickHeight, brickDepth); // center, width, height, depth
}

void CStoneBrickObj::InitializeVertexList(VertexArray2D* vertexList) {
    for (int i = 0; i < m_squareSideLength; i++) {
        std::vector<std::vector<GLdouble>> newRow;

        for (int j = 0; j < m_squareSideLength; j++) {
            std::vector<GLdouble> defaultValues = { 1, 2, 3, 0 }; // 0 at the end marks the vertex as not fixed
            newRow.push_back(defaultValues);
        }
        (*vertexList).push_back(newRow);
    }
}

double CStoneBrickObj::GetRand(double min, double max) {
    return min + ((((double)rand()) / RAND_MAX) * (max - min));
}

GLdouble CStoneBrickObj::GetDepthFromCoord(VertexArray2D* vertexList, XYCoord coord) {
    return (*vertexList)[coord.x][coord.y][2];
}

GLdouble CStoneBrickObj::GetAverageDepth(VertexArray2D* vertexList, XYCoord topLeft, XYCoord bottomLeft, XYCoord bottomRight, XYCoord topRight, bool fourthCoord) {
    GLdouble result = 0;
    result += GetDepthFromCoord(vertexList, topLeft);
    result += GetDepthFromCoord(vertexList, bottomLeft);
    result += GetDepthFromCoord(vertexList, bottomRight);
    if (fourthCoord) result += GetDepthFromCoord(vertexList, topRight);
    return (result / (fourthCoord ? 4 : 3)) + m_postFrontDepthBoost;
}


void CStoneBrickObj::CalculateSquareSidePoint(VertexArray2D* vertexList, XYCoord myIndex, XYCoord pointA, XYCoord pointB, XYCoord otherPoint, int squareSideLength) {
    // the point to calculate lies between point A and pointB
    // other point is the 3rd point not on the A-B axis. It's used with A and B to find the average depth.

    // if marked as fixed return
    if ((*vertexList)[myIndex.x][myIndex.y][3] == 1) return;

    XYCoord potentiallyUnusedCoordinate(2 * myIndex.x - otherPoint.x, 2 * myIndex.y - otherPoint.y);
    bool useThisCoord = (potentiallyUnusedCoordinate.x > 0 && potentiallyUnusedCoordinate.x < squareSideLength) &&
        (potentiallyUnusedCoordinate.y > 0 && potentiallyUnusedCoordinate.y < squareSideLength);
    useThisCoord = false;

    (*vertexList)[myIndex.x][myIndex.y][0] = ((*vertexList)[pointA.x][pointA.y][0] + (*vertexList)[pointB.x][pointB.y][0]) / 2; // x component
    (*vertexList)[myIndex.x][myIndex.y][1] = ((*vertexList)[pointA.x][pointA.y][1] + (*vertexList)[pointB.x][pointB.y][1]) / 2; // y component

    // check if the last point is out of bounds
    (*vertexList)[myIndex.x][myIndex.y][2] = GetAverageDepth(vertexList, pointA, pointB, otherPoint, potentiallyUnusedCoordinate, useThisCoord); // z component

    // give x and y a bit of randomness
    double xyJitterRange = 0.1;
    (*vertexList)[myIndex.x][myIndex.y][0] += GetRand(-m_wallXYJitterRange, m_wallXYJitterRange);
    (*vertexList)[myIndex.x][myIndex.y][1] += GetRand(-m_wallXYJitterRange, m_wallXYJitterRange);
}


void CStoneBrickObj::DiamondSquareIteration(VertexArray2D* vertexList, XYCoord topLeft, XYCoord bottomLeft, XYCoord bottomRight, XYCoord topRight, int squareSideLength) {
    //
    // --- DIAMOND STEP ---
    //

    // figure out the center point coordinates
    XYCoord centerIndex((topRight.x + bottomLeft.x) / 2, (topRight.y + bottomLeft.y) / 2); // should be 1, 1

    // compute average height and set the center vertex's values
    if((*vertexList)[centerIndex.x][centerIndex.y][3] != 1) // if not marked as fixed...
    {
        (*vertexList)[centerIndex.x][centerIndex.y][0] = ((*vertexList)[topRight.x][topRight.y][0] + (*vertexList)[bottomLeft.x][bottomLeft.y][0]) / 2; // x component
        (*vertexList)[centerIndex.x][centerIndex.y][1] = ((*vertexList)[topRight.x][topRight.y][1] + (*vertexList)[bottomLeft.x][bottomLeft.y][1]) / 2; // y component
        (*vertexList)[centerIndex.x][centerIndex.y][2] = GetAverageDepth(vertexList, topLeft, bottomLeft, bottomRight, topRight, true); // z component

        // add some randomness to the depth
        (*vertexList)[centerIndex.x][centerIndex.y][2] += GetRand(-m_wallBrickDepthJitterRange, m_wallBrickDepthJitterRange);
    }

    //
    // --- SQUARE STEP ---
    //
    XYCoord middleTop(centerIndex.x, topRight.y);
    CalculateSquareSidePoint(vertexList, middleTop, topRight, topLeft, centerIndex, squareSideLength); // up
    XYCoord rightCenter(topRight.x, centerIndex.y);
    CalculateSquareSidePoint(vertexList, rightCenter, topRight, bottomRight, centerIndex, squareSideLength);// right
    XYCoord middleBottom(centerIndex.x, bottomRight.y);
    CalculateSquareSidePoint(vertexList, middleBottom, bottomLeft, bottomRight, centerIndex, squareSideLength);// down
    XYCoord leftCenter(topLeft.x, centerIndex.y);
    CalculateSquareSidePoint(vertexList, leftCenter, bottomLeft, topLeft, centerIndex, squareSideLength);// left


    //
    // --- RECURSIVE STEP ---
    //

    // check if done using a heuristic
    if (!(middleTop.x - 1 <= topLeft.x))
    {
        // call again with new coords (topLeft, bottomLeft, bottomRight, topRight)
        DiamondSquareIteration(vertexList, topLeft, leftCenter, centerIndex, middleTop, squareSideLength); // top left square
        DiamondSquareIteration(vertexList, leftCenter, bottomLeft, middleBottom, centerIndex, squareSideLength);
        DiamondSquareIteration(vertexList, centerIndex, middleBottom, bottomRight, rightCenter, squareSideLength);
        DiamondSquareIteration(vertexList, middleTop, centerIndex, rightCenter, topRight, squareSideLength);
    }
}

void CStoneBrickObj::SetFourCornersOfVertexArray(VertexArray2D* vertexList, std::vector<GLdouble> topLeft, std::vector<GLdouble> bottomLeft, std::vector<GLdouble> bottomRight, std::vector<GLdouble> topRight) {
    (*vertexList)[0][0] = bottomLeft;
    (*vertexList)[0][m_squareSideLength - 1] = topLeft; // top left
    (*vertexList)[m_squareSideLength - 1][0] = bottomRight; // bottom right
    (*vertexList)[m_squareSideLength - 1][m_squareSideLength - 1] = topRight; // top right
}

void CStoneBrickObj::RotateVertices(VertexArray2D* vertexList, std::vector<GLdouble> pointOfRotation, int axis, double theta) {

    double cosTheta = cos(theta);
    double sinTheta = sin(theta);
    double rotationMatrix[3][3] = { {1, 0, 0}, {0, cosTheta, -sinTheta}, {0, sinTheta, cosTheta} }; // x axis

    if (axis == 1) { // y axis
        double newRotationMatrix[3][3] = { {cosTheta, 0, sinTheta}, {0, 1, 0}, {-sinTheta, 0, cosTheta} };
        memcpy(rotationMatrix, newRotationMatrix, sizeof(rotationMatrix));
    }
    else if (axis == 2) { // z axis
        double newRotationMatrix[3][3] = { {cosTheta, -sinTheta, 0}, {sinTheta, cosTheta, 0}, {0, 0, 1} };
        memcpy(rotationMatrix, newRotationMatrix, sizeof(rotationMatrix));
    }


    // rotate to top
    for (int i = 0; i < m_squareSideLength; i++) {
        for (int j = 0; j < m_squareSideLength; j++) {

            // get original point and translate to origin
            double x = (*vertexList)[i][j][0] - pointOfRotation[0];
            double y = (*vertexList)[i][j][1] - pointOfRotation[1];
            double z = (*vertexList)[i][j][2] - pointOfRotation[2];

            // rotate and translate back
            (*vertexList)[i][j][0] = (rotationMatrix[0][0] * x + rotationMatrix[0][1] * y + rotationMatrix[0][2] * z) + pointOfRotation[0]; // set x
            (*vertexList)[i][j][1] = (rotationMatrix[1][0] * x + rotationMatrix[1][1] * y + rotationMatrix[1][2] * z) + pointOfRotation[1]; // set y
            (*vertexList)[i][j][2] = (rotationMatrix[2][0] * x + rotationMatrix[2][1] * y + rotationMatrix[2][2] * z) + pointOfRotation[2]; // set z
        }
    }
}

std::vector<GLdouble> GetMidpoint(std::vector<GLdouble> pointA, std::vector<GLdouble> pointB) {
    return { (pointA[0] + pointB[0])/2, (pointA[1] + pointB[1]) / 2, (pointA[2] + pointB[2]) / 2 };
}

std::vector<GLdouble> CStoneBrickObj::GetReferencedValueFixed(int i, VertexArray2D* referenceVertexList, bool RefARow, int rowOrColToRef, bool reverseIterateOneSide)
{
    i = reverseIterateOneSide ? m_squareSideLength-1-i : i;
    return RefARow ? (*referenceVertexList)[i][rowOrColToRef] : (*referenceVertexList)[rowOrColToRef][i];
}

void CStoneBrickObj::SetFixedEdge(bool fixARow, int rowOrColToSet, bool RefARow, int rowOrColToRef, VertexArray2D* referenceVertexList, VertexArray2D* vertexListToFix, bool reverseIterateOneSide) {
    // the new surface will always have row 0 border the front
    for (int i = 0; i < m_squareSideLength; i++) {
        if (fixARow) {
            (*vertexListToFix)[i][rowOrColToSet] = GetReferencedValueFixed(i, referenceVertexList, RefARow, rowOrColToRef, reverseIterateOneSide);
            (*vertexListToFix)[i][rowOrColToSet][3] = 1; // mark as fixed
        }
        else {
            (*vertexListToFix)[rowOrColToSet][i] = GetReferencedValueFixed(i, referenceVertexList, RefARow, rowOrColToRef, reverseIterateOneSide);
            (*vertexListToFix)[rowOrColToSet][i][3] = 1; // mark as fixed
        }
    }
}

std::vector<GLdouble> DoubledVector(std::vector<GLdouble> point, std::vector<GLdouble> vectorOrigin) {
    return { 2*(point[0] - vectorOrigin[0]) + vectorOrigin[0], 2 * (point[1] - vectorOrigin[1]) + vectorOrigin[1], 2 * (point[2] - vectorOrigin[2]) + vectorOrigin[2], 1 };
}

void CStoneBrickObj::InitializeDiamondSquare(VertexArray2D* vertList) {
    DiamondSquareIteration(vertList, XYCoord(0, m_squareSideLength - 1), XYCoord(0, 0), XYCoord(m_squareSideLength - 1, 0), XYCoord(m_squareSideLength - 1, m_squareSideLength - 1), m_squareSideLength);
}

double CStoneBrickObj::RandomBounded(double initial, double max) {
    double result = initial + GetRand(-m_brickShapeJitterRange, m_brickShapeJitterRange);

    //if (result < 0) result = 0; // assuming 0 is the min
    //if (result > max) result = max;

    return result;
}

void CStoneBrickObj::GenerateVertices(std::vector<GLdouble> centerPoint, double width, double height, double depth) {

    // adjust x positions to fit the wall
    double xMin = centerPoint[0] - (width / 2);
    double xMax = centerPoint[0] + (width / 2);
    if (xMin < 0) xMin = 0;
    if (xMax > m_WallWidth) xMax = m_WallWidth;
    if (xMin > xMax || xMin == xMax) return;

    double yInitialLow = centerPoint[1] - (height / 2);
    double yInitialHigh = centerPoint[1] + (height / 2);

    // wall points (counterclockwise)
    std::vector<GLdouble> wallPointA = { 
        RandomBounded(xMin, m_WallWidth), 
        RandomBounded(yInitialHigh, m_WallHeight), 
        centerPoint[2], 
        1 
    };
    std::vector<GLdouble> wallPointB = { 
        RandomBounded(xMin, m_WallWidth), 
        RandomBounded(yInitialLow, m_WallHeight), 
        centerPoint[2], 
        1 
    };
    std::vector<GLdouble> wallPointC = { 
        RandomBounded(xMax, m_WallWidth), 
        RandomBounded(yInitialLow, m_WallHeight), 
        centerPoint[2], 
        1 
    };
    std::vector<GLdouble> wallPointD = { 
        RandomBounded(xMax, m_WallWidth), 
        RandomBounded(yInitialHigh, m_WallHeight), 
        centerPoint[2], 
        1 
    };


    // outward points
    double maxDepthHeight = centerPoint[2] + (depth * 2);
    double initialDepth = centerPoint[2] + depth;

    std::vector<GLdouble> outerPointA = { 
        RandomBounded(xMin, m_WallWidth), 
        RandomBounded(yInitialHigh, m_WallHeight), 
        RandomBounded(initialDepth, maxDepthHeight),
        1 
    }; // add to z to offset balance
    std::vector<GLdouble> outerPointB = { 
        RandomBounded(xMin, m_WallWidth), 
        RandomBounded(yInitialLow, m_WallHeight),
        RandomBounded(initialDepth, maxDepthHeight),
        1 
    };
    std::vector<GLdouble> outerPointC = { 
        RandomBounded(xMax, m_WallWidth), 
        RandomBounded(yInitialLow, m_WallHeight),
        RandomBounded(initialDepth, maxDepthHeight),
        1 
    };
    std::vector<GLdouble> outerPointD = { 
        RandomBounded(xMax, m_WallWidth), 
        RandomBounded(yInitialHigh, m_WallHeight),
        RandomBounded(initialDepth, maxDepthHeight),
        1 
    };


    //
    // diamond square
    // 
    
    // Set the four corners
    SetFourCornersOfVertexArray(&m_vertices, outerPointA, outerPointB, outerPointC, outerPointD); // front
    SetFourCornersOfVertexArray(&m_verticesTop, DoubledVector(wallPointA, outerPointA), outerPointA, outerPointD, DoubledVector(wallPointD, outerPointD)); // top
    SetFourCornersOfVertexArray(&m_verticesLeft, DoubledVector(wallPointB, outerPointB), outerPointB, outerPointA, DoubledVector(wallPointA, outerPointA)); // left
    SetFourCornersOfVertexArray(&m_verticesRight, DoubledVector(wallPointD, outerPointD), outerPointD, outerPointC, DoubledVector(wallPointC, outerPointC)); // right
    SetFourCornersOfVertexArray(&m_verticesBottom, DoubledVector(wallPointC, outerPointC), outerPointC, outerPointB, DoubledVector(wallPointB, outerPointB)); // bottom

    

    // go through full grid populating
    InitializeDiamondSquare(&m_vertices);

    m_postFrontDepthBoost = 0.02;

    // Use the front to set the bottom of the top panel
    SetFixedEdge(true, 0, true, m_squareSideLength - 1, &m_vertices, &m_verticesTop, false);
    RotateVertices(&m_verticesTop, GetMidpoint(wallPointA, wallPointB), 0, PI / 2);

    // populate top panel
    InitializeDiamondSquare(&m_verticesTop);

    // rotate top back
    RotateVertices(&m_verticesTop, GetMidpoint(wallPointA, wallPointB), 0, -PI / 2);

    m_postFrontDepthBoost = -0.02;
    
    // LEFT
    SetFixedEdge(true, 0, false, 0, &m_vertices, &m_verticesLeft, false); // edge between front and left
    SetFixedEdge(false, m_squareSideLength - 1, false, 0, &m_verticesTop, &m_verticesLeft, false); // edge between left and top
    RotateVertices(&m_verticesLeft, GetMidpoint(wallPointA, outerPointB), 1, -PI / 2);
    InitializeDiamondSquare(&m_verticesLeft);
    RotateVertices(&m_verticesLeft, GetMidpoint(wallPointA, outerPointB), 1, PI / 2);


    // RIGHT
    SetFixedEdge(true, 0, false, m_squareSideLength - 1, &m_vertices, &m_verticesRight, true); // edge between front and right
    SetFixedEdge(false, 0, false, m_squareSideLength - 1, &m_verticesTop, &m_verticesRight, false); // edge between right and top
    RotateVertices(&m_verticesRight, GetMidpoint(wallPointD, outerPointC), 1, PI / 2);
    InitializeDiamondSquare(&m_verticesRight);
    RotateVertices(&m_verticesRight, GetMidpoint(wallPointD, outerPointC), 1, -PI / 2);

    m_postFrontDepthBoost = 0.02;

    // Bottom
    SetFixedEdge(true, 0, true, 0, &m_vertices, &m_verticesBottom, true); // edge between front and bottom
    SetFixedEdge(false, m_squareSideLength - 1, false, 0, &m_verticesLeft, &m_verticesBottom, false); // edge between left and bottom
    SetFixedEdge(false, 0, false, m_squareSideLength - 1, &m_verticesRight, &m_verticesBottom, false); // edge between right and bottom
    RotateVertices(&m_verticesBottom, GetMidpoint(wallPointA, wallPointB), 0, -PI / 2);
    InitializeDiamondSquare(&m_verticesBottom);
    RotateVertices(&m_verticesBottom, GetMidpoint(wallPointA, wallPointB), 0, PI / 2);
}

