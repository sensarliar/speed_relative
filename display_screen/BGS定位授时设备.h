// BGS定位授时设备.h : main header file for the BGS定位授时设备 application
//

#if !defined(AFX_BGS_H__A5ECBB5D_7AB6_4B5F_8243_7AE117384EB9__INCLUDED_)
#define AFX_BGS_H__A5ECBB5D_7AB6_4B5F_8243_7AE117384EB9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CBGSApp:
// See BGS定位授时设备.cpp for the implementation of this class
//

class CBGSApp : public CWinApp
{
public:
	CBGSApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBGSApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CBGSApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BGS_H__A5ECBB5D_7AB6_4B5F_8243_7AE117384EB9__INCLUDED_)
