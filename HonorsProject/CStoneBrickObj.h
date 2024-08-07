#pragma once
#include "graphics/OpenGLWnd.h"
#include "graphics/GrTexture.h"
#include "XYCoord.h"
#include <vector>


using VertexArray2D = std::vector<std::vector<std::vector<GLdouble>>>;

class CStoneBrickObj : public COpenGLWnd
{
	// methods
public:
	CStoneBrickObj(double seed, double wallWidth, double wallHeight, std::vector<GLdouble> brickPosition, double brickWidth, double brickHeight, double brickDepth);


	// might just replace bigArray with member variable
	GLdouble GetDepthFromCoord(VertexArray2D* vertexList, XYCoord coord);
	GLdouble GetAverageDepth(VertexArray2D* vertexList, XYCoord topLeft, XYCoord bottomLeft, XYCoord bottomRight, XYCoord topRight, bool fourthCoord);

	void GenerateVertices(std::vector<GLdouble>, double width, double height, double depth);
	
	void DiamondSquareIteration(VertexArray2D* vertexList, XYCoord topLeft, XYCoord bottomLeft, XYCoord bottomRight, XYCoord topRight, int squareSideLength);
	void CalculateSquareSidePoint(VertexArray2D* vertexList, XYCoord myIndex, XYCoord pointA, XYCoord pointB, XYCoord otherPoint, int squareSideLength);

	double GetRand(double min, double max);
	double RandomBounded(double initial, double max);

	void SetFourCornersOfVertexArray(VertexArray2D* vertexList, std::vector<GLdouble> topLeft, std::vector<GLdouble> bottomLeft, std::vector<GLdouble> bottomRight, std::vector<GLdouble> topRight);


	void RotateVertices(VertexArray2D* vertexList, std::vector<GLdouble> pointOfRotation, int axis, double theta);

	void InitializeVertexList(VertexArray2D* vertexList);

	void SetFixedEdge(bool fixARow, int rowOrColToSet, bool RefARow, int rowOrColToRef, VertexArray2D* referenceVertexList, VertexArray2D* vertexListToFix, bool reverseIterateOneSide);

	void InitializeDiamondSquare(VertexArray2D* vertList);

	std::vector<GLdouble> GetReferencedValueFixed(int i, VertexArray2D* referenceVertexList, bool RefARow, int rowOrColToRef, bool reverseIterateOneSide);

	// attributes
public:
	VertexArray2D m_vertices;

	VertexArray2D m_verticesTop;
	VertexArray2D m_verticesLeft;
	VertexArray2D m_verticesRight;
	VertexArray2D m_verticesBottom;

	double m_postFrontDepthBoost;

	double m_WallWidth;
	double m_WallHeight;

	double m_brickShapeJitterRange;
	double m_wallPolygonFactor;
	double m_wallBrickDepthJitterRange;
	double m_wallXYJitterRange;

	int m_squareSideLength;
};

