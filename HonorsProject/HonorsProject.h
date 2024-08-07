
// HonorsProject.h : main header file for the HonorsProject application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CHonorsProjectApp:
// See HonorsProject.cpp for the implementation of this class
//

class CHonorsProjectApp : public CWinApp
{
public:
	CHonorsProjectApp() noexcept;


// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation

public:
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CHonorsProjectApp theApp;
