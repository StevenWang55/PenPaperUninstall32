
// PenPaperUninstall32.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CPenPaperUninstall32App:
// See PenPaperUninstall32.cpp for the implementation of this class
//

class CPenPaperUninstall32App : public CWinApp
{
public:
	CPenPaperUninstall32App();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CPenPaperUninstall32App theApp;