
// ChildView.h : interface of the CChildView class
//


#pragma once
#include "graphics/OpenGLWnd.h"
#include "graphics/GrCamera.h"
#include "graphics/GrTexture.h"
#include "CStoneBrickObj.h"
#include "XYCoord.h"
#include <list>
#include <vector>

// CChildView window


class CChildView : public COpenGLWnd
{
// Construction
public:
	CChildView();

// Attributes
public:
	double m_wallPolygonFactor;
	double m_wallBrickDepthJitterRange;
	double m_wallXYJitterRange;
	
	CGrCamera m_camera;

	double m_WallWidth;
	double m_WallHeight;

	//std::vector<GLdouble*> m_vertices;

	std::vector<CStoneBrickObj*> m_bricks;

	CGrTexture m_testRockTexture;
	CGrTexture m_testWorldTexture;
	CGrTexture m_generatedRockTexture;

// Operations
public:
	void GenerateBricks();
	void XYCoordsToQuad(VertexArray2D* bigArray, XYCoord topLeft, XYCoord bottomLeft, XYCoord bottomRight, XYCoord topRight, int xMax);

	void RenderBrickSurface();

// Overrides
protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
	virtual ~CChildView();
	virtual void OnGLDraw(CDC* pDC);

	// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
};

